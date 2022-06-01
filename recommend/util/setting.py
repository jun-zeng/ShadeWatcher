import argparse
import logging

from colorlog import ColoredFormatter

logger = logging.getLogger(name=__name__)

def init_logger(level: int) -> None:
    """Set logger level indicating which logging statements printed while programs running.
    """
    formatter = ColoredFormatter(
        "%(white)s%(asctime)10s | %(log_color)s%(levelname)6s | %(log_color)s%(message)6s",
        reset=True,
        log_colors={
            'DEBUG':    'cyan',
            'INFO':     'yellow',
            'WARNING':  'green',
            'ERROR':    'red',
            'CRITICAL': 'red,bg_white',
        },
    )

    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(level)

def parse_args() -> argparse.Namespace:
    """Parse user input and configuration setting.
    """
    parser = argparse.ArgumentParser(prog="driver" ,
                                     description="recommendation system")

    # setting for envs
    parser.add_argument('--logging', type=int, default=10,
                        help='Log level[10-50] (default: 10 - Debug)')
    parser.add_argument('--gpu_id', type=str, default='-1',
                        help='GPU device id (default: -1 - CPU)')

    # setting for dataset
    parser.add_argument('--dataset', type=str, default='test',
                        help='Dir to store encoding dataset.')
    parser.add_argument('--ignore_inter', nargs='?', default='[]',
                        help='Ignore system entity interactions in [3,4,132] file.')
    parser.add_argument('--inter_pos_rate', type=int, default=2,
                        help='ratio of positive and negative entities in interactions')
    parser.add_argument('--triple_pos_rate', type=int, default=2,
                        help='ratio of positive and negative entities in embedding')

    # setting for model
    parser.add_argument('--pretrain', type=int, default=0,
                        help='0: No pretrain, 1: Pretrain with kg embeddings, 2:Pretrain with stored models.')
    parser.add_argument('--report', default=False, action='store_true',
                        help='whether report pre-trained model performance.')
    parser.add_argument('--model_type', type=str, default='gnn',
                        help='type of learning model from {gnn}')
    parser.add_argument('--adj_type', type=str, default='si',
                        help='type of adjacency (norm) matrix from {bi, si}')

    # setting for training
    parser.add_argument('--test_size', type=float, default=0.1,
                        help='Size of test dataset')
    parser.add_argument('--val_size', type=float, default=0.1,
                        help='Size of validation dataset')
    
    parser.add_argument('--threshold', type=float, default=1.5,
                        help='threshold to distinguish between benign and malicious interactions')         
    parser.add_argument('--early_stop', default=False, action='store_true',
                        help='early stop according to validation loss')
    parser.add_argument('--epoch', type=int, default=1000,
                        help='Number of epoch')
    parser.add_argument('--lr', type=float, default=0.001,
                        help='Learning rate tuned from [0.0001, 0.001, 0.01]')
    parser.add_argument('--regs', nargs='?', default='[1e-5,1e-5]',
                        help='Regularization for user and item embeddings.')
    parser.add_argument('--opt_type', type=str, default='Adam',
                        help='type of training optimizer from {Adam, SGD, AdaDelta}')
    parser.add_argument('--mess_dropout', nargs='?', default='[0.2,0.2,0.2]',
                        help='drop probability')

    parser.add_argument('--no_gnn', default=False, action='store_true',
                        help='whether using gnn.')
    parser.add_argument('--no_kg', default=False, action='store_true',
                        help='whether using knowledge graph embedding.')
    parser.add_argument('--no_att', default=False, action='store_true',
                        help='whether using attention mechanism.')

    parser.add_argument('--show_test', default=False, action='store_true',
                        help='show test results')
    parser.add_argument('--show_val', default=False, action='store_true',
                        help='show validation results')
    parser.add_argument('--no_step', default=False, action='store_true',
                        help='number of epoch to show training loss')

    parser.add_argument('--save_model', default=False, action='store_true',
                        help='whether save model parameters.')
    
    # setting for kg
    parser.add_argument('--batch_size_kg', type=int, default=1024,
                        help='Embedding batch size')
    parser.add_argument('--embedding_type', type=str, default='transr',
                        help='type of embedding model from {transr,transe,transh}')
    parser.add_argument('--inter_dim', type=int, default=32,
                        help='embedding size for system entities and attributes (interactions) searched from [16, 32, 64]')
    parser.add_argument('--kg_dim', type=int, default=32,
                        help='embedding size for relations (knowledge graph)')
    
    # setting for gnn
    parser.add_argument('--batch_size_gnn', type=int, default=1024,
                        help='Gnn batch size')
    parser.add_argument('--layer_size', nargs='?', default='[32,16]',
                        help='embedding size of every layer (changed with mess_dropout)')
    parser.add_argument('--agg_type', nargs='?', default='graphsage',
                        help='Specify the type of gnn aggregation from {bi, gcn, graphsage}.')

    # advanced option
    parser.add_argument('--train_kg', default=False, action='store_true',
                        help='only train kg embedding')
    parser.add_argument('--train_gnn', default=False, action='store_true',
                        help='only train gnn')
    parser.add_argument('--save_embedding', default=False, action='store_true',
                        help='save kg embedding from weights')

    args = parser.parse_args()

    # init arguments
    if args.train_kg:
        args.no_gnn = True
        args.no_att = True
        args.show_val = True
        args.show_test = True

    if args.train_gnn:
        args.no_kg = True
        args.no_att = True
        args.pretrain = 2

    if args.save_embedding:
        args.report = True
        args.pretrain = 2

    return args

def init_setting() -> argparse.Namespace:
    """Init setting.
    """
    args = parse_args()
    init_logger(args.logging)

    return args
