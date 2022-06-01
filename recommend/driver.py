import os
os.environ['TF_CPP_MIN_LOG_LEVEL']='2'

import warnings
warnings.filterwarnings('ignore', category=FutureWarning)
warnings.filterwarnings('ignore', category=RuntimeWarning)

import random as rd
import numpy as np
import tensorflow as tf

from time import time
from util.setting import init_setting, logger
from util.meta_data import MetaData

from model.GNN import GNN
from util.model_eval import early_stopping, test, validation
from util.helper import ensureDir
from util.data_loader import load_pretrain_embedding, load_data_engine


def main() -> None:
    """Controller of auditing-based recommendation system.

    Control the workflow of modeling training and predicting.
    """

    # set seed for random data
    tf.set_random_seed(2021)
    np.random.seed(2021)
    rd.seed(2021)

    # init setting (user input and logging configuration)
    args = init_setting()

    # define GPU/CPU device to train model
    os.environ['CUDA_VISIBLE_DEVICES'] = args.gpu_id

    # Load Dataset
    meta_data = MetaData(args.dataset)
    data_generator = load_data_engine(args, meta_data)

    # Load pre-trained kg embeddings
    pretrain_embedding = None
    if args.pretrain == 1:
        embedding_save_path = '%s/%s/embedding/%s/%s.npz' % \
                            (meta_data.out_path, args.embedding_type, args.lr, args.model_type)
        pretrain_embedding = load_pretrain_embedding(embedding_save_path)

    # Select learning models
    model = None
    if args.model_type == 'gnn' and args.embedding_type in ['transr', 'transe', 'transh']:
        model = GNN(args=args, meta_data=meta_data, pretrain_embedding=pretrain_embedding)
    else:
        logger.info('the learning model is unknown')
        exit(-1)

    # Save model parameter (weights)
    if args.save_model:
        weight_save_path = None
        save_saver = None
        layer = '-'.join([str(l) for l in eval(args.layer_size)])
        regs = '-'.join([str(r) for r in eval(args.regs)])
        weight_save_path = '%s/%s/%s/weight/%s/_l%s/_r%s/model.weights' % \
                            (meta_data.out_path, args.model_type, args.embedding_type, args.lr, layer, regs)
        ensureDir(weight_save_path)
        save_saver = tf.train.Saver(max_to_keep=1)

    # Setup tensorflow session
    tf_config = tf.ConfigProto()
    tf_config.gpu_options.allow_growth = True
    sess = tf.Session(config=tf_config)
    sess.run(tf.global_variables_initializer())
    
    # Reload model parameters
    if args.pretrain == 2:
        saver = tf.train.Saver()
        layer = '-'.join([str(l) for l in eval(args.layer_size)])
        regs = '-'.join([str(r) for r in eval(args.regs)])
        checkpoint_path = '%s/%s/%s/weight/%s/_l%s/_r%s/checkpoint' % \
                        (meta_data.out_path, args.model_type, args.embedding_type, args.lr, layer, regs)
        ckpt = tf.train.get_checkpoint_state(os.path.dirname(checkpoint_path))
        if ckpt and ckpt.all_model_checkpoint_paths:
            saver.restore(sess, ckpt.all_model_checkpoint_paths[0])
            logger.info("training with pre-training:", os.path.dirname(checkpoint_path))
        else:
            logger.info("training without pre-training")

        # Report pre-trained model performance
        if args.report:
            rel_stat = test(sess, model, data_generator, args.threshold)
            logger.info('test pre-trained model:')
            for metrics, value in rel_stat.items():
                logger.info('metrics: {}, value: {}'.format(metrics, value))

        # Save kg embeddings (independent function)
        if args.save_embedding:
            embedding_save_path = '%s/%s/embedding/%s/%s.npz' % \
                            (meta_data.out_path, args.embedding_type,args.lr, args.model_type)
            ensureDir(embedding_save_path)
            entity_attr_embed = sess.run(
                    [model.weights['entity_attr_embed']], feed_dict={})
            np.savez(embedding_save_path, entity_attr_embed=entity_attr_embed[0])
            logger.info('KG embedding Save in path:\t {}' .format( embedding_save_path))
            exit(0)

    # Training Phase
    logger.info('Total {} epochs'.format(args.epoch))
    logger.info('Epoch X [time]: train==[loss= inter_loss + kg_loss + reg_loss]')
    logger.info('Epoch X [time]: test==[true_positive, false_positive]')

    # parameters for early stop
    stopping_step = 0
    best_tn = 0

    # whether use knowledge-aware attention
    if args.no_att == False:
        model.update_attentive_A(sess)

    for epoch in range(args.epoch):
        t1 = time()
        # inter_loss is gnn/... loss, reg_loss is regularization loss
        loss, inter_loss, kg_loss, reg_loss = 0., 0., 0., 0.

        # phase 1: train the GNN
        if args.no_gnn == False:
            n_batch_gnn = data_generator.n_train_inter // args.batch_size_gnn + 1
            for _ in range(n_batch_gnn):
                # tt1 = time()
                batch_data = data_generator.generate_train_batch()
                feed_dict = data_generator.generate_train_feed_dict(model, batch_data)
                # GNN: batch_loss = batch_inter_loss + batch_reg_loss
                _, batch_loss, batch_inter_loss, batch_reg_loss = model.train_inter(sess, feed_dict)

                loss += batch_loss
                inter_loss += batch_inter_loss
                reg_loss += batch_reg_loss

            if np.isnan(loss) == True:
                logger.error('error: loss@gnn is nan')
                exit(-1)

        # phase 2: train the KG embedding (e.g., TransR)
        if args.no_kg == False:
            n_batch_kg = len(data_generator.all_h_list) // args.batch_size_kg + 1 
            for _ in range(n_batch_kg):
                batch_data = data_generator.generate_train_kg_batch()
                feed_dict = data_generator.generate_train_kg_feed_dict(model, batch_data)
                # TransR: batch_loss = batch_kg_loss + batch_reg_loss
                _, batch_loss, batch_kg_loss, batch_reg_loss = model.train_kg(sess, feed_dict)

                loss += batch_loss
                kg_loss += batch_kg_loss
                reg_loss += batch_reg_loss

            if np.isnan(loss) == True:
                logger.error('error: loss@kg embedding is nan')
                exit(-1)

        # print training loss
        if args.no_step == False:
            perf_train_ite = 'Epoch %d [%.1fs]: train==[%.5f = %.5f + %.5f + %.5f]' \
                       % (epoch + 1, time() - t1, loss, inter_loss, kg_loss, reg_loss)
            logger.info(perf_train_ite)

        # phase 3: model validation
        if args.show_val:
            t2 = time()
            rel = validation(sess, model, data_generator, args.threshold)
            perf_val_benign = 'validation==[[%.1fs] tn_b: %d, fp_b: %d]' % (time() - t2, rel['tn_b'], rel['fp_b'])
            logger.info(perf_val_benign)

            # Early stop if #tn_b does not change or decrease for XX successive steps
            if args.early_stop: 
                best_tn, stopping_step, should_stop = early_stopping(rel['tn_b'],
                                                                    best_tn, 
                                                                    stopping_step,
                                                                    flag_step=5)
                # stopping_step == 0 represents less missing threats
                if stopping_step == 0:
                    if args.save_model:
                        t4 = time()
                        save_saver.save(sess, weight_save_path, global_step=epoch)
                        logger.info('Model Save [%.1fs] in path: %s' % (time() - t4, weight_save_path))
                if should_stop:
                    break

    # Testing Phase
    if args.show_test:
        rel_stat = test(sess, model, data_generator, args.threshold)
        logger.info('test model:')
        for metrics, value in rel_stat.items():
            logger.info('metrics: {}, value: {}'.format(metrics, value))

    # Save model parameters
    t3 = time()
    if args.save_model and args.early_stop == False:
        save_saver.save(sess, weight_save_path, global_step=epoch)
        logger.info('Model Save [%.1fs] in path: %s' % (time() - t3, weight_save_path))

if __name__ == '__main__':
    main()
