import collections
import random as rd

import numpy as np
import scipy.sparse as sp
from model.GNN import GNN

from util.base_data import DataBase
from util.setting import logger


class GnnLoader(DataBase):
    """Maintaining gnn data basing on DataBase class.

    Processing and Preparing data for GNN model.
    """
    def __init__(self, args):
        super().__init__(args)

        # generate sparse adjacency matrices for system entity inter_train_data & relation_dict
        adj_list, self.adj_r_list = self._get_relational_adj_list()

        # generate normalized (sparse adjacency) matrices for A_in
        self.norm_list = self._get_relational_norm_list(adj_list)

        # generate kg triples dict, key is 'head', value is '(tail, relation)'
        self.all_kg_dict = self._get_all_kg_dict()
        self.exist_head = list(self.all_kg_dict.keys())
        self.exist_head_size = len(self.exist_head)

        # generate sorted kg triples list: head, relation, tail, value
        self.all_h_list, self.all_r_list, self.all_t_list, self.all_v_list = self._get_all_kg_data()

    def _get_all_kg_data(self) -> tuple:
        """Sorting knowledge graph indices to satisfy tensorflow sparse matrix operations.
        """
        def _reorder_list(org_list, order):
            new_list = np.array(org_list)
            new_list = new_list[order]
            return new_list

        all_h_list, all_t_list, all_r_list, all_v_list = [], [], [], []

        for l_id, norm in enumerate(self.norm_list):
            all_h_list += list(norm.row)
            all_t_list += list(norm.col)
            # norm.data stores A_in
            all_v_list += list(norm.data)
            all_r_list += [self.adj_r_list[l_id]] * len(norm.row)

        assert len(all_h_list) == sum([len(norm.data) for norm in self.norm_list])

        # tensorflow.sparse.softmax in GNN requires indices sorted in the canonical
        # lexicographic order, so that we sort kg triples
        logger.info('start sorting indices in kg triples...')
        org_h_dict = dict()

        for idx, h in enumerate(all_h_list):
            if h not in org_h_dict.keys():
                org_h_dict[h] = [[], [], []]

            org_h_dict[h][0].append(all_t_list[idx])
            org_h_dict[h][1].append(all_r_list[idx])
            org_h_dict[h][2].append(all_v_list[idx])
        logger.debug('reorganizing kg data done.')

        # sort tail for every head in sorted_h_dict
        sorted_h_dict = dict()
        for h in org_h_dict.keys():
            org_t_list, org_r_list, org_v_list = org_h_dict[h]
            sort_t_list = np.array(org_t_list)
            sort_order = np.argsort(sort_t_list)

            sort_t_list = _reorder_list(org_t_list, sort_order)
            sort_r_list = _reorder_list(org_r_list, sort_order)
            sort_v_list = _reorder_list(org_v_list, sort_order)

            sorted_h_dict[h] = [sort_t_list, sort_r_list, sort_v_list]
        logger.debug('sorting (tail,relation) in heads done')

        # sort head in sorted_h_dict
        od = collections.OrderedDict(sorted(sorted_h_dict.items()))

        new_h_list, new_t_list, new_r_list, new_v_list = [], [], [], []
        for h, vals in od.items():
            new_h_list += [h] * len(vals[0])
            new_t_list += list(vals[0])
            new_r_list += list(vals[1])
            new_v_list += list(vals[2])

        assert sum(new_h_list) == sum(all_h_list)
        assert sum(new_t_list) == sum(all_t_list)
        assert sum(new_r_list) == sum(all_r_list)
        logger.debug('sorting head done.')
        logger.info('finish sorting indices in kg triples')

        return new_h_list, new_r_list, new_t_list, new_v_list

    def _get_all_kg_dict(self) -> collections.defaultdict:
        """Generating knowledge graph triples dict.

        key: head, value: (tail, relation)
        """
        all_kg_dict = collections.defaultdict(list)

        for l_id, norm in enumerate(self.norm_list):
            rows = norm.row
            cols = norm.col
            for i_id in range(len(rows)):
                head = rows[i_id]
                tail = cols[i_id]
                relation = self.adj_r_list[l_id]

                all_kg_dict[head].append((tail, relation))

        return all_kg_dict

    def _get_relational_norm_list(self, adj_list: list) -> list:
        """Generating normalized matrices for sparse adjacency in adj_list.
        """

        # Init for 1/Nt
        def _si_norm(adj):
            rowsum = np.array(adj.sum(axis=1))
            # It is reasonable for np.power(rowsum, -1).flatten() to trigger divide by zero encountered warning
            d_inv = np.power(rowsum, -1).flatten()
            d_inv[np.isinf(d_inv)] = 0.
            d_mat_inv = sp.diags(d_inv)
            norm_adj = d_mat_inv.dot(adj)

            return norm_adj.tocoo()

        # Init for 1/(Nt*Nh)^(1/2)
        def _bi_norm(adj):
            rowsum = np.array(adj.sum(axis=1))
            d_inv_sqrt = np.power(rowsum, -0.5).flatten()
            d_inv_sqrt[np.isinf(d_inv_sqrt)] = 0.
            d_mat_inv_sqrt = sp.diags(d_inv_sqrt)
            bi_norm = d_mat_inv_sqrt.dot(adj).dot(d_mat_inv_sqrt)

            return bi_norm.tocoo()
        logger.info('start generating normalized adjacency matrix with {}...' .format(self.args.adj_type))
        if self.args.adj_type == 'bi':
            norm_list = [_bi_norm(adj) for adj in adj_list]
            logger.debug('generating bi-normalized adjacency matrix done.')
        else:
            logger.debug('generating si-normalized adjacency matrix done.')
            norm_list = [_si_norm(adj) for adj in adj_list]
        
        logger.info('finish generating normalized adjacency matrix.')

        return norm_list

    def _get_relational_adj_list(self) -> tuple:
        """Generating sparse adjacency matrices for system entity inter_train_data & relation_dict
        """
        def _np_mat2sp_adj(np_mat, row_pre=0, col_pre=0):
            n_all = self.n_entity_attr

            # to-node interaction: A: A->B
            a_rows = np_mat[:, 0] + row_pre # all As
            a_cols = np_mat[:, 1] + col_pre # all Bs

            # must use float 1. (int 1 is not allowed)
            a_vals = [1.] * len(a_rows)

            # from-node interaction: A: B->A
            b_rows = a_cols
            b_cols = a_rows
            b_vals = [1.] * len(b_rows)

            a_adj = sp.coo_matrix((a_vals, (a_rows, a_cols)), shape=(n_all, n_all))
            b_adj = sp.coo_matrix((b_vals, (b_rows, b_cols)), shape=(n_all, n_all))

            return a_adj, b_adj

        adj_mat_list = []
        adj_r_list = []

        logger.info('start converting graph info into sparse adjacency matrix...')
        # Todo: (Optional) r_inv, k_inv are for inverse directions (e.g., ActedBy for ActorOf)
        r, r_inv = _np_mat2sp_adj(self.inter_train_data)
        adj_mat_list.append(r)
        adj_r_list.append(0)
        # adj_mat_list.append(r_inv)
        # adj_r_list.append(self.n_relation + 1)

        logger.debug('converting system interactions into sparse adjacency matrix done.')

        for r_id in self.relation_dict.keys():
            k, k_inv = _np_mat2sp_adj(np.array(self.relation_dict[r_id]))
            adj_mat_list.append(k)
            adj_r_list.append(r_id + 1)
            # adj_mat_list.append(k_inv)
            # adj_r_list.append(r_id + 1 + self.n_relation + 1)

        logger.debug('converting knowledge graph into sparse adjacency matrix done.')

        logger.info('finish converting graph info into sparse adjacency matrix.')

        # n_relations = (n_relations + 1) * 2 if inverse relations are enabled
        self.n_relations = len(adj_r_list)

        return adj_mat_list, adj_r_list

    def generate_train_batch(self) -> dict:
        """Generating training batch of system interactions for GNN.
        """
        batch_data = {}
        e_batch, pos_e_batch, neg_e_batch = self._generate_train_inter_batch()
        batch_data['e_batch'] = e_batch
        batch_data['pos_e_batch'] = pos_e_batch
        batch_data['neg_e_batch'] = neg_e_batch

        return batch_data

    def generate_test_batch(self, i_batch: int) -> dict:
        """Generating testing batch of system interactions for GNN.
        """
        batch_data = {}
        start = i_batch * self.batch_size_test
        if i_batch == self.n_batch_test - 1:
            end = self.n_test_inter
        else:
            end = (i_batch + 1) * self.batch_size_test
        batch_data['e_batch'] = self.inter_test_e[start: end]
        batch_data['neg_e_batch'] = self.inter_test_neg[start: end]

        return batch_data

    def generate_val_batch(self, i_batch: int) -> dict:
        """Generating validating batch of system interactions for GNN.
        """
        batch_data = {}
        start = i_batch * self.batch_size_val
        if i_batch == self.n_batch_val - 1:
            end = self.n_val_inter
        else:
            end = (i_batch + 1) * self.batch_size_val
        batch_data['e_batch'] = self.inter_val_e[start: end]
        batch_data['neg_e_batch'] = self.inter_val_neg[start: end]

        return batch_data

    def generate_train_kg_batch(self):
        """Generating training batch of system interaction for KG embedding (e.g., TransR)"""
        batch_data = {}
        h_batch, r_batch, pos_t_batch, neg_t_batch = self._generate_train_kg_batch()
        batch_data['h_batch'] = h_batch
        batch_data['r_batch'] = r_batch
        batch_data['pos_t_batch'] = pos_t_batch
        batch_data['neg_t_batch'] = neg_t_batch

        return batch_data

    def _generate_train_kg_batch(self) -> tuple:
        """Sampling system interactions for kg training (e.g., TransR).
        """
        def sample_neg_triple_for_h(neg_triples):
            n_neg_triples = len(neg_triples)
            neg_id = np.random.randint(low=0, high=n_neg_triples)
            neg_r = neg_triples[neg_id][1]
            neg_t = neg_triples[neg_id][0]

            return neg_r, neg_t

        def sample_pos_triple_for_h(neg_triples, r, rate):
            pos_t = []
            while len(pos_t) != rate:
                t = np.random.randint(low=0, high=self.n_entity)
                if (t, r) not in neg_triples and t not in pos_t:
                    pos_t.append(t)

            return pos_t

        if self.batch_size_kg <= self.exist_head_size:
            h_batch = np.array(rd.sample(self.exist_head, self.batch_size_kg))
        else:
            h_batch = np.array([rd.choice(self.exist_head) for _ in range(self.batch_size_kg)])

        r_batch = np.zeros(shape=[self.batch_size_kg])
        neg_t_batch = np.zeros(shape=[self.batch_size_kg])
        pos_t_batch = np.zeros(shape=[self.batch_size_kg * self.triple_pos_rate])

        for idx, h in enumerate(h_batch):
            neg_triples = self.all_kg_dict[h]

            neg_r, neg_t = sample_neg_triple_for_h(neg_triples)
            r_batch[idx] = neg_r
            neg_t_batch[idx] = neg_t

            pos_t_list = sample_pos_triple_for_h(neg_triples, neg_r, self.triple_pos_rate)
            for pos_idx, pos_t in enumerate(pos_t_list):
                pos_t_batch[idx * self.triple_pos_rate + pos_idx] = pos_t

        return h_batch, r_batch, pos_t_batch, neg_t_batch

    def generate_train_feed_dict(self, model: GNN, batch_data: dict) -> dict:
        """Generating feed dict for GNN model training.
        """
        feed_dict = {
            model.e: batch_data['e_batch'],
            model.pos_e: batch_data['pos_e_batch'],
            model.neg_e: batch_data['neg_e_batch'],
            model.mess_dropout: eval(self.args.mess_dropout)
        }

        return feed_dict

    def generate_train_kg_feed_dict(self, model: GNN, batch_data: dict) -> dict:
        """Generating feed dict for kg embedding training
        """
        feed_dict = {
            model.h: batch_data['h_batch'],
            model.r: batch_data['r_batch'],
            model.pos_t: batch_data['pos_t_batch'],
            model.neg_t: batch_data['neg_t_batch']
        }
        return feed_dict

    def generate_test_val_feed_dict(self, model: GNN, batch_data: dict)-> dict:
        """Generating testing and validating feed dict.
        """
        feed_dict = {
            model.e: batch_data['e_batch'],
            model.neg_e: batch_data['neg_e_batch'],
            
            # hardcode dropping probability
            model.mess_dropout: [0,0,0,0,0,0]
        }

        return feed_dict

    @staticmethod
    def generate_test_threat_data(inter_file: str) -> dict:
        """For evaluation: generating threat data.
        """
        inter_threat = np.loadtxt(inter_file, dtype=int)
        inter_size = np.size(inter_threat, 0)
        
        e = np.full(inter_size, inter_threat[0][0])
        inter_e = np.zeros(shape=inter_size)
        label_e = np.zeros(shape=inter_size)
        for idx, inter in enumerate(inter_threat):
            inter_e[idx] = inter[1]
            label_e[idx] = inter[2]

        threat_data = {}
        threat_data['e'] = e
        threat_data['threat_e'] = inter_e
        threat_data['label_e'] = label_e

        return threat_data

    @staticmethod
    def generate_test_threat_feed_dict(model: GNN, threat_data: dict) -> dict:
        """For evaluation: generating treat data feed dict.
        """
        feed_dict = {
            model.e: threat_data['e'],
            model.pos_e: threat_data['threat_e'],

            # hardcode dropping probability
            model.mess_dropout: [0,0,0,0,0,0]
        }
        return feed_dict
