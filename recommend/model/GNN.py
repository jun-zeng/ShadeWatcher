from math import log
import warnings
warnings.filterwarnings('ignore', category=FutureWarning)
import os
os.environ['TF_CPP_MIN_LOG_LEVEL']='2'
import numpy as np
import scipy.sparse as sp
import tensorflow as tf
from util.meta_data import MetaData
import argparse
from util.setting import logger


class GNN(object):
    """Recommendation model based-on GNN.

    Demonstrating the detail of audit-based detection model, (e.g., network structure, propagation mechanism).
    """
    def __init__(self, args: argparse.Namespace, meta_data: MetaData, pretrain_embedding: np.array=None) -> None:
        """Init GNN class with args, meta_data, and pretrain_embedding
        """
        logger.info("start initing Graph Neural Network...")
        self._parse_args(args, meta_data, pretrain_embedding)

        # create placeholder for training inputs
        self._build_inputs()

        # create Variable for training weights
        if args.gpu_id == '-1':
        	self._build_weights(args.embedding_type)
        else:
        	with tf.device("/device:GPU:0"):
        		self._build_weights(args.embedding_type)

        # compute Graph-based Representation via Graph Neural Network
        self._build_inter_model()

        # optimize Recommendation (gnn) via BPR loss
        self._build_inter_loss()

        # compute Knowledge Graph Embeddings via TransR/TransE/TransH
        if args.embedding_type == 'transr':
            self._build_transr_model()
        elif args.embedding_type == 'transe':
            self._build_transe_model()
        elif args.embedding_type == 'transh':
            self._build_transh_model()

        # optimize TransR via BPR loss
        self._build_kg_loss()

        # count #parameters (weights)
        self._statistics_params()

    def _parse_args(self, args: argparse.Namespace, meta_data: MetaData, pretrain_embedding: np.array) -> None:
        """Parsing user inputs and meta for GNN model.
        """
        self.model_type = 'gnn'

        # pretrain kg embeddings
        self.pretrain_embedding = pretrain_embedding

        # data config
        self.n_entity = meta_data.n_entity
        self.n_attr = meta_data.n_attr
        self.n_relation = meta_data.n_relation
        self.n_entity_attr = meta_data.n_entity_attr

        # Todo: Set up n_fold for training; may not need n_fold
        self.n_fold = 1
        if self.n_entity_attr < self.n_fold:
            self.n_fold = 1

        # init attentive matrix A for phase I
        self.A_in = meta_data.A_in

        # knowledge graph triplets
        self.all_h_list = meta_data.all_h_list
        self.all_r_list = meta_data.all_r_list
        self.all_t_list = meta_data.all_t_list
        self.all_v_list = meta_data.all_v_list

        # setting for system entity interactions
        self.inter_dim = args.inter_dim
        self.batch_size_gnn = args.batch_size_gnn
        self.weight_size = eval(args.layer_size)
        self.n_layer = len(self.weight_size)
        self.agg_type = args.agg_type

        # setting for kg
        self.kg_dim = args.kg_dim
        self.batch_size_kg = args.batch_size_kg
        self.margin = 1.0

        # setting for training
        self.regs = eval(args.regs)
        self.opt_type = args.opt_type
        self.lr = args.lr
        self.triple_pos_rate = args.triple_pos_rate
        self.inter_pos_rate = args.inter_pos_rate

        self.model_type += '_%s_%s_l%d' % (args.adj_type, self.agg_type, self.n_layer)

    def _build_inputs(self):
        """Building inputs for interaction, kg, dropout, and attention
        """
        logger.info('start building inputs')
        # inputs for interactions
        self.e = tf.placeholder(tf.int64, shape=[None], name='e')
        self.pos_e = tf.placeholder(tf.int64, shape=[None], name='pos_e')
        self.neg_e = tf.placeholder(tf.int64, shape=[None], name='neg_e')

        # inputs for kg
        self.h = tf.placeholder(tf.int64, shape=[None], name='h')
        self.r = tf.placeholder(tf.int64, shape=[None], name='r')
        self.pos_t = tf.placeholder(tf.int64, shape=[None], name='pos_t')
        self.neg_t = tf.placeholder(tf.int64, shape=[None], name='neg_t')

        # dropout: message dropout (adopted on the convolution operations)
        # Todo: add node dropout
        self.mess_dropout = tf.placeholder(tf.float32, shape=[None], name='mess_dropout')

        # Inputs for attention-aware knowledge update
        self.A_values = tf.placeholder(tf.float32, shape=[len(self.all_v_list)], name='A_values')
        logger.info('finish building inputs')

    def _build_weights(self, embedding_type: str) -> None:
        """Building weights placeholder for model parameters.
        """
        logger.info('start building weights')
        all_weight = dict()

        initializer = tf.contrib.layers.xavier_initializer(seed=2021)

        # kg embeddings
        if self.pretrain_embedding is None:
            all_weight['entity_attr_embed'] = tf.Variable(initializer([self.n_entity_attr, self.inter_dim]), name='entity_attr_embed')
            logger.info('adapting Xavier to initialize kg embedding')
        else:
            all_weight['entity_attr_embed'] = tf.Variable(initial_value=self.pretrain_embedding['entity_attr_embed'], trainable=True, name='entity_attr_embed', dtype=tf.float32)
            logger.info('adapting Pre-train results to initialize kg embedding')

        logger.info('adapting Xavier to initialize relation embedding')
        all_weight['rel_embed'] = tf.Variable(initializer([self.n_relation + 1, self.kg_dim]), name='rel_embed')
        
        # transformation matrix for TransR
        logger.info('adapting Xavier to initialize TransR embedding')
        all_weight['trans_w'] = tf.Variable(initializer([self.n_relation + 1, self.inter_dim, self.kg_dim]), name='trans_w')
        
        # hyperplane matrix for TransH
        if embedding_type == 'transh':
            logger.info('adapting Xavier to initialize transH embedding')
            all_weight['trans_h'] = tf.Variable(initializer([self.n_relation + 1, self.kg_dim]), name='trans_h')

        weight_size_list = [self.inter_dim] + self.weight_size

        """ Different Convolutional Layer:
        1. bi:'KGAT: Knowledge Graph Attention Network for Recommendation', KDD'2019
        2. gcn: 'Semi-Supervised Classification with Graph Convolutional Networks', ICLR'2018
        3. graphsage: 'Inductive Representation Learning on Large Graphs', NeurIPS2017. 
        """
        # weights for gnn
        for k in range(self.n_layer):
            # Gcn aggregator
            if self.agg_type in ['gcn']:
                logger.info('adapting Xavier to initialize GCN embedding')
                all_weight['w_gcn_%d' % k] = tf.Variable(initializer([weight_size_list[k], weight_size_list[k + 1]]), name='w_gcn_%d' % k)
                all_weight['b_gcn_%d' % k] = tf.Variable(initializer([1, weight_size_list[k + 1]]), name='b_gcn_%d' % k)

            # Graphsage aggregator
            elif self.agg_type in ['graphsage']:
                logger.info('adapting Xavier to initialize GraphSage embedding')
                all_weight['w_sage_%d' % k] = tf.Variable(initializer([2 * weight_size_list[k], weight_size_list[k + 1]]), name='w_sage_%d' % k)
                all_weight['b_sage_%d' % k] = tf.Variable(initializer([1, weight_size_list[k + 1]]), name='b_sage_%d' % k)

            # Bi-Interaction aggregator
            elif self.agg_type in ['bi']:
                logger.info('adapting Xavier to initialize Bi-Interaction embedding')
                all_weight['w_bi_%d' % k] = tf.Variable(initializer([weight_size_list[k], weight_size_list[k + 1]]), name='w_bi_%d' % k)
                all_weight['b_bi_%d' % k] = tf.Variable(initializer([1, weight_size_list[k + 1]]), name='b_bi_%d' % k)
            
            else:
                logger.error('graph aggregator type is unknown')
                exit(-1)

        self.weights = all_weight
        logger.info('finish building weights')

    def _build_inter_model(self) -> None:
        """Building recommendation model network and propagating.
        """
        logger.info('start building inter model')
        if self.agg_type in ['bi']:
            self.ea_embedding = self._create_bi_inter_embed()
        elif self.agg_type in ['gcn']:
            self.ea_embedding = self._create_gcn_embed()
        elif self.agg_type in ['graphsage']:
            self.ea_embedding = self._create_graphsage_embed()
        else:
            logger.error('graph aggregator type is unknown')
            exit(-1)
        
        # lookup embeddings for entity and its positive and negative interactions
        self.e_e = tf.nn.embedding_lookup(self.ea_embedding, self.e)
        self.pos_e_e = tf.nn.embedding_lookup(self.ea_embedding, self.pos_e)
        self.neg_e_e = tf.nn.embedding_lookup(self.ea_embedding, self.neg_e)

        # prediction
        self.batch_predictions = tf.matmul(self.e_e, self.neg_e_e, transpose_a=False, transpose_b=True)
        
        logger.info('finish building inter model')

    def _create_gcn_embed(self) -> tf.Tensor:
        """Creating GCN network
        """
        logger.info('start building GCN network')
        # attention matrix for Knowledge-aware Attention
        A = self.A_in

        # generate a set of adjacency sub-matrix.
        A_fold_hat = self._split_A_hat(A)

        # previous embedding (before update)
        pre_embedding = self.weights['entity_attr_embed']
        ea_embeddings = [pre_embedding]

        for k in range(self.n_layer):
            # sum embeddings of neighbors
            temp_embed = []
            if self.n_fold == 1:
                neighbor_embedding = tf.sparse_tensor_dense_matmul(A_fold_hat[0], pre_embedding, name='gcn_neighbor_{}'.format(k))
            else:
                for f in range(self.n_fold):
                    temp_embed.append(tf.sparse_tensor_dense_matmul(A_fold_hat[f], pre_embedding))
                neighbor_embedding = tf.concat(temp_embed, 0, name='gcn_neighbor_{}'.format(k))

            # LeakyReLU (W1(eh + eNh))
            pre_embedding = neighbor_embedding + pre_embedding
            pre_embedding = tf.nn.leaky_relu(
                tf.matmul(pre_embedding, self.weights['w_gcn_%d' % k]) + self.weights['b_gcn_%d' % k], name='gcn_embed_{}'.format(k))

            # dropout for overfitting mitigation
            pre_embedding = tf.nn.dropout(pre_embedding, 1 - self.mess_dropout[k], name='gcn_dropout_{}'.format(k))
            # normalize the distribution of embedding
            norm_embeddings = tf.math.l2_normalize(pre_embedding, axis=1, name='gcn_norm_{}'.format(k))

            ea_embeddings += [norm_embeddings]

        ea_embeddings = tf.concat(ea_embeddings, 1)

        logger.info('finish building GCN network')

        return ea_embeddings

    def _create_graphsage_embed(self) -> tf.Tensor:
        """Creating GraphSage network
        """
        logger.info('start building GraphSage network')
        # attention matrix for Knowledge-aware Attention
        A = self.A_in

        # generate a set of adjacency sub-matrix.
        A_fold_hat = self._split_A_hat(A)

        # previous embedding (before update) 
        pre_embedding = self.weights['entity_attr_embed']
        ea_embeddings = [pre_embedding]

        for k in range(self.n_layer):
            # line 1 in algorithm 1 [RM-GCN, KDD'2018], aggregator layer: weighted sum
            temp_embed = []
            if self.n_fold == 1:
                neighbor_embedding = tf.sparse_tensor_dense_matmul(A_fold_hat[0], pre_embedding, name='sage_neighbor_{}'.format(k))
            else:
                for f in range(self.n_fold):
                    temp_embed.append(tf.sparse_tensor_dense_matmul(A_fold_hat[f], pre_embedding))
                neighbor_embedding = tf.concat(temp_embed, 0, name='sage_neighbor_{}'.format(k))

            # line 2 in algorithm 1 [RM-GCN, KDD'2018], aggregating the previous embedding
            neighbor_embedding = tf.concat([pre_embedding, neighbor_embedding], 1)

            # LeakyReLU (W1(eh || eNh))
            pre_embedding = tf.nn.leaky_relu(
                tf.matmul(neighbor_embedding, self.weights['w_sage_%d' % k]) + self.weights['b_sage_%d' % k])

            pre_embedding = tf.nn.dropout(pre_embedding, 1 - self.mess_dropout[k])

            # normalize the distribution of embeddings.
            norm_embeddings = tf.math.l2_normalize(pre_embedding, axis=1)

            ea_embeddings += [norm_embeddings]

        ea_embeddings = tf.concat(ea_embeddings, 1)
        logger.info('finish building GraphSage network')

        return ea_embeddings

    def _create_bi_inter_embed(self) -> tf.Tensor:
        """Creating bi-inter network
        """
        logger.info('start building Bi-Inter network')
        # attention matrix for Knowledge-aware Attention
        A = self.A_in

        # generate a set of adjacency sub-matrix for memory efficiency (hat => head attention)
        A_fold_hat = self._split_A_hat(A)

        # previous embedding (before update)
        pre_embedding = self.weights['entity_attr_embed']
        ea_embeddings = [pre_embedding]

        for k in range(self.n_layer):
            # sum embeddings of neighbors
            temp_embed = []
            if self.n_fold == 1:
                neighbor_embedding = tf.sparse_tensor_dense_matmul(A_fold_hat[0], pre_embedding, name='bi_neighbor_{}'.format(k))
            else:
                for f in range(self.n_fold):
                    temp_embed.append(tf.sparse_tensor_dense_matmul(A_fold_hat[f], pre_embedding))
                neighbor_embedding = tf.concat(temp_embed, 0, name='bi_neighbor_{}'.format(k))


            # LeakyReLU (W1(eh + eNh))
            add_embedding = neighbor_embedding + pre_embedding
            sum_embedding = tf.nn.leaky_relu(
                tf.matmul(add_embedding, self.weights['w_bi_%d'%k])+self.weights['b_bi_%d'%k]
            )
            
            # LeakyReLU (W2(eh ⊙ eNh))
            dot_embedding = tf.multiply(neighbor_embedding, pre_embedding)
            bi_embedding = tf.nn.leaky_relu(
                tf.matmul(dot_embedding, self.weights['w_bi_%d'%k])+self.weights['b_bi_%d'%k]
            )

            pre_embedding = sum_embedding + bi_embedding
            # dropout for overfitting mitigation
            pre_embedding = tf.nn.dropout(pre_embedding, 1 - self.mess_dropout[k])

            # normalize the distribution of embeddings
            norm_embedding = tf.math.l2_normalize(pre_embedding, axis=1)
            ea_embeddings += [norm_embedding]

        ea_embeddings = tf.concat(ea_embeddings, 1)

        logger.info('finish building Bi-Inter network')

        return ea_embeddings

    def _build_inter_loss(self) -> None:
        """Building inter model loss function
        """
        logger.info('start building inter loss')
        current_batch_size = tf.shape(self.e_e)[0]
        neg_scores = tf.reduce_sum(tf.multiply(self.e_e, self.neg_e_e), axis=1, name='gnn_neg_scores')

        pos_scores = tf.zeros([current_batch_size], name='gnn_pos_scores')
        for i in range(self.inter_pos_rate):
            # generate positive sub batch to calculate pos_scores
            _pos_e_e = self.pos_e_e[i::self.inter_pos_rate]
            pos_scores += tf.reduce_sum(tf.multiply(self.e_e, _pos_e_e), axis=1)

        # regularization for overfitting mitigation
        regularizer = tf.nn.l2_loss(self.e_e) + tf.nn.l2_loss(self.pos_e_e) + \
                      tf.nn.l2_loss(self.neg_e_e)
        regularizer = regularizer / tf.cast(current_batch_size, dtype=tf.float32)

        # Using softplus to implement BPR loss
        # negatives are benign interactions; and positives are malicious interactions 
        inter_loss = tf.reduce_mean(tf.nn.softplus(neg_scores - pos_scores), name='gnn_loss')

        self.inter_loss = inter_loss
        self.reg_loss = self.regs[0] * regularizer
        self.loss = self.inter_loss + self.reg_loss

        # Optimization
        logger.info('adapting {} as optimization function for gnn' .format(self.opt_type))
        if self.opt_type in ['Adam', 'adam']:
            self.opt = tf.train.AdamOptimizer(learning_rate=self.lr).minimize(self.loss)
        elif self.opt_type in ['SGD', 'sgd']:
            self.opt = tf.train.GradientDescentOptimizer(learning_rate=self.lr).minimize(self.loss)
        elif self.opt_type in ['AdaDelta']:
            self.opt = tf.train.AdadeltaOptimizer(learning_rate=self.lr).minimize(self.loss)
        else:
            logger.error('Optimizer is unknown')
            exit(-1)

        logger.info('finish building inter loss')

    def _split_A_hat(self, A: list) -> list:
        """Splitting attention matrix list according to n_fold
        """
        A_fold_hat = []
        fold_len = self.n_entity_attr // self.n_fold

        for i_fold in range(self.n_fold):
            start = i_fold * fold_len
            if i_fold == self.n_fold - 1:
                end = self.n_entity_attr
            else:
                end = (i_fold + 1) * fold_len
            A_fold_hat.append(self._convert_sp_mat_to_sp_tensor(A[start:end]))

        return A_fold_hat

    def _convert_sp_mat_to_sp_tensor(self, X: sp.spmatrix) -> tf.SparseTensor:
        """Converting sp sparse matrix to tensor sparse matrit
        """
        coo = X.tocoo().astype(np.float32)
        indices = np.mat([coo.row, coo.col]).transpose()

        if len(coo.data) > 0:
            return tf.SparseTensor(indices, coo.data, coo.shape)
        else:
            return tf.SparseTensor(indices=np.empty((0,2), dtype=np.int64), values=coo.data, dense_shape=coo.shape)

    def _build_transr_model(self) -> None:
        """Creating TransR model
        """
        logger.info('start building TransR model.')
        self.h_e, self.r_e, self.pos_t_e, self.neg_t_e = self._get_transr_inference(self.h, self.r, self.pos_t, self.neg_t)
        self.A_kg_score = self._generate_transR_score(h=self.h, t=self.neg_t, r=self.r)
        self.A_out = self._create_attentive_A_out()
        logger.info('finish building TransR model.')

    def _build_transe_model(self) -> None:
        """Creating TransE model
        """
        logger.info('start building TransE model.')
        self.h_e, self.r_e, self.pos_t_e, self.neg_t_e = self._get_transe_inference(self.h, self.r, self.pos_t, self.neg_t)
        self.A_kg_score = self._generate_transE_score(h=self.h, t=self.neg_t, r=self.r)
        self.A_out = self._create_attentive_A_out()
        logger.info('finish building TransE model.')

    def _build_transh_model(self) -> None:
        """Creating TransH model
        """
        logger.info('start building TransH model.')
        self.h_e, self.r_e, self.pos_t_e, self.neg_t_e = self._get_transh_inference(self.h, self.r, self.pos_t, self.neg_t)
        self.A_kg_score = self._generate_transH_score(h=self.h, t=self.neg_t, r=self.r)
        self.A_out = self._create_attentive_A_out()
        logger.info('finish building TransH model.')

    def _create_attentive_A_out(self) -> tf.SparseTensor:
        """Creating attentive A sparse tensor.
        """
        indices = np.mat([self.all_h_list, self.all_t_list]).transpose()
        # normalize the coefficients across triplets
        return tf.sparse.softmax(tf.SparseTensor(indices, self.A_values, self.A_in.shape))

    def _generate_transE_score(self, h: tf.Tensor, t: tf.Tensor, r: tf.Tensor) -> tf.Tensor:
        """Calculating TransE score
        """
        embedding = self.weights['entity_attr_embed']

        # head and tail embeddings: batch_size * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        t_e = tf.nn.embedding_lookup(embedding, t)

        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)

        kg_score = tf.reduce_sum(tf.multiply(t_e, tf.tanh(h_e + r_e)), axis=1)

        return kg_score
    
    def _generate_transH_score(self, h: tf.Tensor, t: tf.Tensor, r: tf.Tensor) -> tf.Tensor:
        """Calculating TransH score
        """
        def _transfer(e, n):
            n = tf.nn.l2_normalize(n, -1)
            return e - tf.reduce_sum(e * n, -1, keepdims = True) * n

        embedding = self.weights['entity_attr_embed']

        # head and tail embeddings: batch_size * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        t_e = tf.nn.embedding_lookup(embedding, t)

        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)

        # hyperplane: batch_size * kg_dim
        norm_e = tf.nn.embedding_lookup(self.weights['trans_h'], r)

        # hyperplane transformation
        h_e = _transfer(h_e, norm_e)
        t_e = _transfer(t_e, norm_e)

        h_e = tf.nn.l2_normalize(h_e, -1)
        t_e = tf.nn.l2_normalize(t_e, -1)
        r_e = tf.nn.l2_normalize(r_e, -1)

        kg_score = tf.reduce_sum(tf.multiply(t_e, tf.tanh(h_e + r_e)), axis=1)

        return kg_score

    def _generate_transR_score(self, h: tf.Tensor, t: tf.Tensor, r: tf.Tensor) -> tf.Tensor:
        """Calculating TransR score.
        """
        embedding = self.weights['entity_attr_embed']
        embedding = tf.expand_dims(embedding, 1)

        # head and tail embeddings: batch_size * 1 * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        t_e = tf.nn.embedding_lookup(embedding, t)

        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)

        # relation transform: batch_size * inter_dim * kg_dim
        trans_r = tf.nn.embedding_lookup(self.weights['trans_w'], r)

        # [batch_size, 1, inter_dim] * [batch_size, inter_dim, kg_dim] == [batch_size, 1, kg_dim]
        h_e = tf.reshape(tf.matmul(h_e, trans_r), [-1, self.kg_dim])
        t_e = tf.reshape(tf.matmul(t_e, trans_r), [-1, self.kg_dim])

        kg_score = tf.reduce_sum(tf.multiply(t_e, tf.tanh(h_e + r_e)), axis=1)

        return kg_score

    def _get_transe_inference(self, h: tf.Tensor, r: tf.Tensor, pos_t: tf.Tensor, neg_t: tf.Tensor) -> tuple:
        """Getting TransE embedding results.
        """
        embedding = self.weights['entity_attr_embed']
        # head and tail embeddings: batch_size * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        pos_t_e = tf.nn.embedding_lookup(embedding, pos_t)
        neg_t_e = tf.nn.embedding_lookup(embedding, neg_t)
        
        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)

        pos_t_e_tmp = []
        for i in range(self.triple_pos_rate):
            # generate positive sub batch to calculate pos_scores
            pos_t_e_tmp.append(pos_t_e[i::self.triple_pos_rate])
        pos_t_e = tf.concat(pos_t_e_tmp, 0)

        return h_e, r_e, pos_t_e, neg_t_e

    def _get_transh_inference(self, h: tf.Tensor, r: tf.Tensor, pos_t: tf.Tensor, neg_t: tf.Tensor) -> tuple:
        """Getting TransH embedding results.
        """
        def _transfer(e, n):
            n = tf.nn.l2_normalize(n, -1)
            return e - tf.reduce_sum(e * n, -1, keepdims = True) * n

        embedding = self.weights['entity_attr_embed']
        # head and tail embeddings: batch_size * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        pos_t_e = tf.nn.embedding_lookup(embedding, pos_t)
        neg_t_e = tf.nn.embedding_lookup(embedding, neg_t)

        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)

        # hyperplane: batch_size * kg_dim
        norm_e = tf.nn.embedding_lookup(self.weights['trans_h'], r)

        # hyperplane transformation
        h_e = _transfer(h_e, norm_e)
        neg_t_e = _transfer(neg_t_e, norm_e)

        h_e = tf.nn.l2_normalize(h_e, -1)
        neg_t_e = tf.nn.l2_normalize(neg_t_e, -1)
        r_e = tf.nn.l2_normalize(r_e, -1)
        
        pos_t_e_tmp = []
        for i in range(self.triple_pos_rate):
            # generate positive sub batch to calculate pos_scores
            _pos_t_e = pos_t_e[i::self.triple_pos_rate]
            _pos_t_e = _transfer(_pos_t_e, norm_e)
            _pos_t_e = tf.nn.l2_normalize(_pos_t_e, -1)
            pos_t_e_tmp.append(_pos_t_e)
        pos_t_e = tf.concat(pos_t_e_tmp, 0)

        return h_e, r_e, pos_t_e, neg_t_e

    def _get_transr_inference(self, h: tf.Tensor, r: tf.Tensor, pos_t: tf.Tensor, neg_t: tf.Tensor) -> tuple:
        """Getting TransR embedding results.
        """
        embedding = self.weights['entity_attr_embed']
        embedding = tf.expand_dims(embedding, 1)
        # head and tail embeddings: batch_size * 1 * inter_dim
        h_e = tf.nn.embedding_lookup(embedding, h)
        pos_t_e = tf.nn.embedding_lookup(embedding, pos_t)
        neg_t_e = tf.nn.embedding_lookup(embedding, neg_t)
        self.h_e_raw = tf.reshape(h_e, [-1, self.kg_dim])
        
        # relation embeddings: batch_size * kg_dim
        r_e = tf.nn.embedding_lookup(self.weights['rel_embed'], r)
        
        # relation transform: batch_size * inter_dim * kg_dim
        trans_r = tf.nn.embedding_lookup(self.weights['trans_w'], r)

        # [batch_size, 1, inter_dim] * [batch_size, inter_dim, kg_dim] == [batch_size, 1, kg_dim]
        h_e = tf.reshape(tf.matmul(h_e, trans_r), [-1, self.kg_dim])
        neg_t_e = tf.reshape(tf.matmul(neg_t_e, trans_r), [-1, self.kg_dim])

        pos_t_e_tmp = []
        for i in range(self.triple_pos_rate):
            # generate positive sub batch to calculate pos_scores
            _pos_t_e = pos_t_e[i::self.triple_pos_rate]
            pos_t_e_tmp.append(tf.reshape(tf.matmul(_pos_t_e, trans_r), [-1, self.kg_dim]))
        pos_t_e = tf.concat(pos_t_e_tmp, 0)

        return h_e, r_e, pos_t_e, neg_t_e

    def _build_kg_loss(self):
        """Building kg embedding loss.
        """
        def _get_kg_score(h_e, r_e, t_e):
            kg_score = tf.reduce_sum(tf.square(h_e + r_e - t_e), axis=1, keep_dims=True)

            return kg_score

        logger.info('start building kg loss.')
        current_batch_size = tf.shape(self.h_e)[0]
        neg_kg_score = _get_kg_score(self.h_e, self.r_e, self.neg_t_e)
        pos_kg_score = tf.zeros([current_batch_size, 1])
        for i in range(self.triple_pos_rate):
            # generate positive sub batch to calculate pos_kg_score
            _pos_t_e = self.pos_t_e[i::self.triple_pos_rate]
            pos_kg_score += _get_kg_score(self.h_e, self.r_e, _pos_t_e)

        # Using softplus to implement BPR loss
        # negatives are valid triplets; and positives are corrupted triplets 
        kg_loss = tf.reduce_mean(tf.nn.softplus(neg_kg_score - pos_kg_score), name='kg_loss')

        regularizer = tf.nn.l2_loss(self.h_e) + tf.nn.l2_loss(self.r_e) + \
                      tf.nn.l2_loss(self.pos_t_e) + tf.nn.l2_loss(self.neg_t_e)
        regularizer = regularizer / tf.cast(current_batch_size, dtype=tf.float32)

        self.kg_loss = kg_loss
        self.reg_loss2 = self.regs[1] * regularizer
        self.loss2 = self.kg_loss + self.reg_loss2

        # Optimization
        logger.info('adapting {} as optimization function for kg.' .format(self.opt_type))
        if self.opt_type in ['Adam', 'adam']:
            self.opt2 = tf.train.AdamOptimizer(learning_rate=self.lr).minimize(self.loss2)
        elif self.opt_type in ['SGD', 'sgd']:
            self.opt2 = tf.train.GradientDescentOptimizer(learning_rate=self.lr).minimize(self.loss2)
        elif self.opt_type in ['AdaDelta']:
            self.opt2 = tf.train.AdadeltaOptimizer(learning_rate=self.lr).minimize(self.loss2)
        else:
            logger.error('optimizer is unknown')
            exit(-1)

        logger.info('finish building kg loss.')

    def _statistics_params(self) -> None:
        """Printing model parameters.
        """
        logger.info('Model parameters:')
        total_parameters = 0
        for var in self.weights:
            shape = self.weights[var].get_shape()
            var_para = 1
            for dim in shape:
                var_para *= dim.value
            logger.info('{}:\t{}' .format(var, var_para))
            total_parameters += var_para

        logger.info('#Total parameters:\t %d' % total_parameters)

    def train_inter(self, sess: tf.Session, feed_dict: dict) -> tuple:
        run_options = tf.RunOptions(report_tensor_allocations_upon_oom = True)
        return sess.run([self.opt, self.loss, self.inter_loss, self.reg_loss], feed_dict, options=run_options)

    def train_kg(self, sess: tf.Session, feed_dict: dict) -> tuple:
        run_options = tf.RunOptions(report_tensor_allocations_upon_oom = True)

        return sess.run([self.opt2, self.loss2, self.kg_loss, self.reg_loss2], feed_dict, options=run_options)

    def eval(self, sess: tf.Session, feed_dict: dict) -> tuple:
        return sess.run(self.batch_predictions, feed_dict)

    def eval_attention(self, sess: tf.Session) -> tuple:
        return sess.run([self.A_vector, self.A_value])

    def embedding(self, sess: tf.Session, feed_dict_r: dict, feed_dict_e: dict) -> tuple:
        trans_r = sess.run(self.h_e, feed_dict_r)
        trans_e = sess.run(self.h_e_raw, feed_dict_e)
        return trans_r, trans_e

    def update_attentive_A(self, sess: tf.Session) -> None:
        """Updating attention matrix
        
        Attention depends on kg score instead of inter score.
        Todo: design an end-to-end pipeline, where the effects of interactions can backpropagate to system entity embeddings.
        """
        n_fold = 50
        fold_len = len(self.all_h_list) // n_fold
        kg_score = []

        for i_fold in range(n_fold):
            start = i_fold * fold_len
            if i_fold == n_fold - 1:
                end = len(self.all_h_list)
            else:
                end = (i_fold + 1) * fold_len

            feed_dict = {
                self.h: self.all_h_list[start:end],
                self.r: self.all_r_list[start:end],
                self.neg_t: self.all_t_list[start:end] 
            }

            A_kg_score = sess.run(self.A_kg_score, feed_dict=feed_dict)
            kg_score += list(A_kg_score)

        kg_score = np.array(kg_score)

        new_A = sess.run(self.A_out, feed_dict={self.A_values: kg_score})
        new_A_values = new_A.values
        new_A_indices = new_A.indices

        rows = new_A_indices[:, 0]
        cols = new_A_indices[:, 1]

        self.A_in = sp.coo_matrix((new_A_values, (rows, cols)), 
                                  shape=(self.n_entity_attr, self.n_entity_attr))
                                  
        self.A_vector = np.vstack((rows, cols)).T
        self.A_value = new_A_values
