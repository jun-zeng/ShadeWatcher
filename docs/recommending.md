# Recommendation System

ShadeWatcher models both first-order and high-order information to detect cyber threats.
In particular, we employ TransR to distill first-order information and GNN to extract high-order relationships.

To validate the effectiveness of ShadeWatcher, we provide different training strategies.
Namely, you can train ShadeWatcher with (1) first-order modeling; (2) high-order
modeling; and (3) first-order and high-order modeling. 

## How to Use

Make sure you have successfully set up the virtual environment for training the
recommendation model under  `ShadeWatcher/recommend`.

1. Show usage or help
```bash
workon shadewatcher
(shadewatcher) python driver.py -h
```
```bash
usage: driver [-h] [--logging LOGGING] [--gpu_id GPU_ID] [--dataset DATASET]
              [--ignore_inter [IGNORE_INTER]]
              [--inter_pos_rate INTER_POS_RATE]
              [--triple_pos_rate TRIPLE_POS_RATE] [--pretrain PRETRAIN]
              [--report] [--model_type MODEL_TYPE] [--adj_type ADJ_TYPE]
              [--test_size TEST_SIZE] [--val_size VAL_SIZE]
              [--threshold THRESHOLD] [--early_stop] [--epoch EPOCH] [--lr LR]
              [--regs [REGS]] [--opt_type OPT_TYPE]
              [--mess_dropout [MESS_DROPOUT]] [--no_gnn] [--no_kg] [--no_att]
              [--show_test] [--show_val] [--no_step] [--save_model]
              [--batch_size_kg BATCH_SIZE_KG]
              [--embedding_type EMBEDDING_TYPE] [--inter_dim INTER_DIM]
              [--kg_dim KG_DIM] [--batch_size_gnn BATCH_SIZE_GNN]
              [--layer_size [LAYER_SIZE]] [--agg_type [AGG_TYPE]] [--train_kg]
              [--train_gnn] [--save_embedding]
```
1. Train model with both first-order and high-order information with epochs 1000
```bash
(shadewatcher) python driver.py --dataset test --epoch 1000 --show_val --show_test
```
Expected Result:
```
2021-11-24 19:40:07,734 |   INFO | System interactions
2021-11-24 19:40:07,734 |  DEBUG | [n_inter]=[621]
2021-11-24 19:40:07,734 |  DEBUG | [n_train_inter, n_test_inter, n_val_inter]=[496, 62, 63]
2021-11-24 19:40:07,734 |   INFO | Knowledge graph
2021-11-24 19:40:07,734 |  DEBUG | [n_triple]=[5945]
2021-11-24 19:40:07,734 |  DEBUG | [n_entity, n_relation and n_attr]=[529, 27, 0]
2021-11-24 19:40:07,734 |   INFO | Detection model
2021-11-24 19:40:07,734 |  DEBUG | [training epoch]=[1000]
2021-11-24 19:40:07,734 |  DEBUG | [batch_size_gnn and batch_size_kg]=[1024, 1024]
2021-11-24 19:40:07,734 |   INFO | start converting graph info into sparse adjacency matrix...
2021-11-24 19:40:07,734 |  DEBUG | converting system interactions into sparse adjacency matrix done.
2021-11-24 19:40:07,738 |  DEBUG | converting knowledge graph into sparse adjacency matrix done.
2021-11-24 19:40:07,738 |   INFO | finish converting graph info into sparse adjacency matrix.
2021-11-24 19:40:07,738 |   INFO | start generating normalized adjacency matrix with si...
2021-11-24 19:40:07,738 |  DEBUG | generating si-normalized adjacency matrix done.
2021-11-24 19:40:07,744 |   INFO | finish generating normalized adjacency matrix.
2021-11-24 19:40:07,746 |   INFO | start sorting indices in kg triples...
2021-11-24 19:40:07,748 |  DEBUG | reorganizing kg data done.
2021-11-24 19:40:07,751 |  DEBUG | sorting (tail,relation) in heads done
2021-11-24 19:40:07,755 |  DEBUG | sorting head done.
2021-11-24 19:40:07,755 |   INFO | finish sorting indices in kg triples
2021-11-24 19:40:07,758 |   INFO | start initing Graph Neural Network...
2021-11-24 19:40:07,758 |   INFO | start building inputs
2021-11-24 19:40:07,761 |   INFO | finish building inputs
2021-11-24 19:40:07,761 |   INFO | start building weights
2021-11-24 19:40:07,968 |   INFO | adapting Xavier to initialize kg embedding
2021-11-24 19:40:07,968 |   INFO | adapting Xavier to initialize relation embedding
2021-11-24 19:40:07,971 |   INFO | adapting Xavier to initialize TransR embedding
2021-11-24 19:40:07,975 |   INFO | adapting Xavier to initialize GraphSage embedding
2021-11-24 19:40:07,982 |   INFO | adapting Xavier to initialize GraphSage embedding
2021-11-24 19:40:07,990 |   INFO | finish building weights
2021-11-24 19:40:07,990 |   INFO | start building inter model
2021-11-24 19:40:07,990 |   INFO | start building GraphSage network
2021-11-24 19:40:08,017 |   INFO | finish building GraphSage network
2021-11-24 19:40:08,021 |   INFO | finish building inter model
2021-11-24 19:40:08,022 |   INFO | start building inter loss
2021-11-24 19:40:08,077 |   INFO | adapting Adam as optimization function for gnn
2021-11-24 19:40:08,330 |   INFO | finish building inter loss
2021-11-24 19:40:08,330 |   INFO | start building TransR model.
2021-11-24 19:40:08,355 |   INFO | finish building TransR model.
2021-11-24 19:40:08,355 |   INFO | start building kg loss.
2021-11-24 19:40:08,373 |   INFO | adapting Adam as optimization function for kg.
2021-11-24 19:40:08,575 |   INFO | finish building kg loss.
2021-11-24 19:40:08,575 |   INFO | Model parameters:
2021-11-24 19:40:08,575 |   INFO | entity_attr_embed:	16928
2021-11-24 19:40:08,576 |   INFO | rel_embed:	864
2021-11-24 19:40:08,576 |   INFO | trans_w:	27648
2021-11-24 19:40:08,576 |   INFO | w_sage_0:	2048
2021-11-24 19:40:08,576 |   INFO | b_sage_0:	32
2021-11-24 19:40:08,576 |   INFO | w_sage_1:	1024
2021-11-24 19:40:08,576 |   INFO | b_sage_1:	16
2021-11-24 19:40:08,576 |   INFO | #Total parameters:	 48560
2021-11-24 19:40:08,649 |   INFO | Total 1000 epochs
2021-11-24 19:40:08,649 |   INFO | Epoch X [time]: train==[loss= inter_loss + kg_loss + reg_loss]
2021-11-24 19:40:08,649 |   INFO | Epoch X [time]: test==[true_positive, false_positive]
2021-11-24 19:40:09,111 |   INFO | Epoch 1 [0.4s]: train==[1.70588 = 0.25196 + 1.45385 + 0.00007]
2021-11-24 19:40:09,133 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:09,337 |   INFO | Epoch 2 [0.2s]: train==[1.59942 = 0.23510 + 1.36424 + 0.00007]
2021-11-24 19:40:09,338 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:09,550 |   INFO | Epoch 3 [0.2s]: train==[1.49074 = 0.23833 + 1.25234 + 0.00007]
2021-11-24 19:40:09,551 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:09,758 |   INFO | Epoch 4 [0.2s]: train==[1.36515 = 0.23452 + 1.13055 + 0.00007]
2021-11-24 19:40:09,759 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:09,950 |   INFO | Epoch 5 [0.2s]: train==[1.23494 = 0.23554 + 0.99933 + 0.00008]
2021-11-24 19:40:09,951 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:10,156 |   INFO | Epoch 6 [0.2s]: train==[1.09465 = 0.22376 + 0.87081 + 0.00008]
2021-11-24 19:40:10,158 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:10,370 |   INFO | Epoch 7 [0.2s]: train==[0.97111 = 0.23016 + 0.74087 + 0.00008]
2021-11-24 19:40:10,372 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:10,578 |   INFO | Epoch 8 [0.2s]: train==[0.82360 = 0.21932 + 0.60421 + 0.00008]
2021-11-24 19:40:10,579 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:10,782 |   INFO | Epoch 9 [0.2s]: train==[0.70983 = 0.23115 + 0.47860 + 0.00008]
2021-11-24 19:40:10,783 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
2021-11-24 19:40:10,980 |   INFO | Epoch 10 [0.2s]: train==[0.60386 = 0.22486 + 0.37891 + 0.00009]
2021-11-24 19:40:10,981 |   INFO | validation==[[0.0s] tn_b: 0, fp_b: 63]
...
2021-11-24 19:43:40,872 |   INFO | Epoch 996 [0.2s]: train==[0.00424 = 0.00384 + 0.00006 + 0.00034]
2021-11-24 19:43:40,874 |   INFO | validation==[[0.0s] tn_b: 55, fp_b: 8]
2021-11-24 19:43:41,097 |   INFO | Epoch 997 [0.2s]: train==[0.00518 = 0.00471 + 0.00013 + 0.00034]
2021-11-24 19:43:41,098 |   INFO | validation==[[0.0s] tn_b: 55, fp_b: 8]
2021-11-24 19:43:41,319 |   INFO | Epoch 998 [0.2s]: train==[0.00421 = 0.00367 + 0.00020 + 0.00034]
2021-11-24 19:43:41,320 |   INFO | validation==[[0.0s] tn_b: 55, fp_b: 8]
2021-11-24 19:43:41,558 |   INFO | Epoch 999 [0.2s]: train==[0.00468 = 0.00420 + 0.00014 + 0.00034]
2021-11-24 19:43:41,560 |   INFO | validation==[[0.0s] tn_b: 55, fp_b: 8]
2021-11-24 19:43:41,783 |   INFO | Epoch 1000 [0.2s]: train==[0.00469 = 0.00427 + 0.00007 + 0.00034]
2021-11-24 19:43:41,784 |   INFO | validation==[[0.0s] tn_b: 55, fp_b: 8]
2021-11-24 19:43:41,785 |   INFO | test model:
2021-11-24 19:43:41,785 |   INFO | metrics: tn_b, value: 55
2021-11-24 19:43:41,785 |   INFO | metrics: fp_b, value: 7
```