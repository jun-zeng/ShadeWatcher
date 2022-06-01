class MetaData(object):
    """Meta data of audit-based recommendation system.

    Attributes:
        in_path: A string indicating one-hot encoding data path.
        out_path: A string indicating embedding result path.
        n_entity: An integer indicating the number of system entities.
        n_attr: An integer indicating the number of attributes.
        n_relation: An integer indicating the number of relation types (e.g., read and location)
        n_triple: An integer indicating the number of edges in knowledge graph.
        n_entity_attr: An integer indicating the number of nodes in knowledge graph.
        A_in: A list storing the knowledge-aware attention matrix.
        all_h_list, all_t_list, all_r_list, all_v_list: lists representing triples of knowledge graph (head, tail, relation, value).
    """
    def __init__(self, dataset: str) -> None:
        """Init class MetaData with dataset.
        """
        self.in_path = '../data/encoding/' + dataset
        self.out_path = '../data/embedding/' + dataset

        self.n_entity = 0
        self.n_attr = 0
        self.n_relation = 0
        self.n_triple = 0
        self.n_entity_attr = 0

        self.A_in = []

        self.all_h_list = []
        self.all_t_list = []
        self.all_r_list = []
        self.all_v_list = []
