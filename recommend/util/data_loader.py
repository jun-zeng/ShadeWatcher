import numpy as np

from util.gnn_data import GnnLoader
from util.setting import logger
from util.meta_data import MetaData


def load_pretrain_embedding(embedding_save_path: str) -> np.array:
    """Loading pretrain embedding array.
    """
    try:
        pretrain_embedding = np.load(embedding_save_path)
    except Exception:
        pretrain_embedding = None

    return pretrain_embedding

def load_data_engine(args, meta_data: MetaData) -> GnnLoader:
    """Load GNN data engine and initialize meta data.
    """
    if args.model_type == 'gnn':
        data_generator = GnnLoader(args)
        meta_data.n_entity = data_generator.n_entity
        meta_data.n_relation = data_generator.n_relation
        meta_data.n_attr = data_generator.n_attr
        meta_data.n_entity_attr = data_generator.n_entity_attr
        meta_data.n_triple = data_generator.n_triple
        meta_data.n_inter = data_generator.n_inter

        # load the norm matrix (used for Knowledge-aware Attention)
        # sum is used to integrate inter_data and kg_data
        meta_data.A_in = sum(data_generator.norm_list)

        # load head, relation, tail triples
        meta_data.all_h_list = data_generator.all_h_list
        meta_data.all_r_list = data_generator.all_r_list
        meta_data.all_t_list = data_generator.all_t_list
        meta_data.all_v_list = data_generator.all_v_list
    else:
        logger.error('the learning model is unknown')
        exit(-1)

    return data_generator
