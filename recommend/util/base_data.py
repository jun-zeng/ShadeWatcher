import os
import numpy as np
import collections
import random as rd
from sklearn.model_selection import train_test_split
from util.setting import logger


class DataBase(object):
    """Maintaining base data for audit-based recommendation system.

    Parsing and maintaining base data info initially,

    Attributes:
        path: A string indicating encoding data file path.
        args: Argparse Namespace preserving all info of user inputs.
        batch_size_gnn: An integer indicating the batch size of gnn.
        batch_size_kg: An integer indicating the batch size of kg embedding.
        batch_size_test: An integer indicating the size of each testing batch.
        batch_size_val: An integer indicating the size of each validation batch.
        inter_pos_rate: An integer indicating the ratio of positive to negative entities in interactions.
        triple_pos_rate: An integer indicating the ratio of positive to negative entities in embedding.
        kg_file: A string indicating train2id file path.
        rel_file: A string indicating relation2id file path.
        entity_file: A string indicating entity2id file path.
        attr_file: A string indicating attr2id file path.
        test_size: A float indicating the ratio of test dataset.
        val_size: A float indicating the ration of validation dataset.
        inter_file: A list including all interaction files excluding ignore interactions.
        ...
    """
    def __init__(self, args) -> None:
        """Init DataBase class with args namespace.
        """
        self.path = '../data/encoding/' + args.dataset
        self.args = args
        self.batch_size_gnn = args.batch_size_gnn
        self.batch_size_kg = args.batch_size_kg
        self.batch_size_test = self.batch_size_gnn * 2
        self.batch_size_val = self.batch_size_gnn * 2
        self.inter_pos_rate = args.inter_pos_rate
        self.triple_pos_rate = args.triple_pos_rate
        self.kg_file = self.path + '/train2id.txt'
        self.rel_file = self.path + '/relation2id.txt'
        self.entity_file = self.path + '/entity2id.txt'
        self.attr_file = self.path + '/attr2id.txt'
        self.test_size = args.test_size
        self.val_size = args.val_size
        self.inter_file = self._traverse_inter_file(args.ignore_inter)

        # system entity interactions for gnn
        self.n_inter = 0
        inter_data, self.inter_dict = self._load_ratings()
        self.exist_entity = list(self.inter_dict.keys())
        self.exist_entity_size = len(self.exist_entity)

        # split inter_data into training data, validation data, and testing data
        self.n_train_inter, self.n_test_inter, self.n_val_inter = 0, 0, 0
        self.inter_train_data, inter_test_data, inter_val_data = self._train_test_split(inter_data)

        # inter_val_e: system entities,  inter_val_neg: negative items
        self.n_batch_test, self.n_batch_val = 0, 0
        self.inter_val_e, self.inter_val_neg  = self._get_val_data(inter_val_data)
        self.inter_test_e, self.inter_test_neg  = self._get_test_data(inter_test_data)

        # knowledge graph for translation-based embedding (e.g., TransR)
        self.n_entity, self.n_attr, self.n_relation, self.n_triple = self._load_kg_stat()
        self.n_entity_attr = self.n_entity + self.n_attr
        self.relation_dict = self._load_kg()

        # log statistic info about the dataset
        self._log_data_info()

    def _traverse_inter_file(self, ignore_inter: str) -> list:
        """Traverse all files and filter interaction files.
        """
        inter_file = []
        ignore_files = eval(ignore_inter)
        ignore_files = ['inter2id_'+str(x)+'.txt' for x in ignore_files]
        for _, _, files in os.walk(self.path):
            for file in files:
                for ignore_file in ignore_files:
                    if file.find(ignore_file) != -1:
                        continue
                if 'inter2id' in file:
                    inter_file.append(self.path + '/' + file)
        return inter_file

    def _train_test_split(self, inter_data: np.array) -> tuple:
        """Splitting interaction data into training, validating, and testing parts.
        """     
        inter_train_data, inter_test_val_data = train_test_split(inter_data, test_size=self.test_size + self.val_size, random_state=2021)
        inter_test_data, inter_val_data = train_test_split(inter_test_val_data, test_size=self.val_size/(self.test_size + self.val_size), random_state=2021)
        self.n_train_inter = len(inter_train_data)
        self.n_test_inter = len(inter_test_data)
        self.n_val_inter = len(inter_val_data)

        return inter_train_data, inter_test_data, inter_val_data

    def _log_data_info(self) -> None:
        """Logging data and model information. 
        """
        logger.info('System interactions')
        logger.debug('[n_inter]=[%d]' % self.n_inter)
        logger.debug('[n_train_inter, n_test_inter, n_val_inter]=[%d, %d, %d]' % (self.n_train_inter, self.n_test_inter, self.n_val_inter))

        logger.info('Knowledge graph')
        logger.debug('[n_triple]=[%d]' % self.n_triple)
        logger.debug('[n_entity, n_relation and n_attr]=[%d, %d, %d]' % (self.n_entity, self.n_relation, self.n_attr))
        logger.info('Detection model')
        logger.debug('[training epoch]=[%d]' % self.args.epoch)
        logger.debug('[batch_size_gnn and batch_size_kg]=[%d, %d]' % (self.batch_size_gnn, self.batch_size_kg))

    def _load_kg_stat(self) -> tuple:
        """Loading knowledge graph meta information.
        """
        n_attr = 0

        with open(self.entity_file, 'r') as f:
            n_entity = int(f.readline().strip())

        with open(self.rel_file, 'r') as f:
            n_relation = int(f.readline().strip())

        with open(self.kg_file, 'r') as f:
            n_triple = int(f.readline().strip())

        return n_entity, n_attr, n_relation, n_triple

    def _load_kg(self) -> collections.defaultdict:
        """Loading knowledge graph.
        """
        def _construct_kg(kg_np):
            rd = collections.defaultdict(list)
            for head, tail, relation in kg_np:
                rd[relation].append((head, tail))

            return rd

        kg_np = np.loadtxt(self.kg_file, dtype=np.int64, skiprows=1)
        kg_np = np.unique(kg_np, axis=0)
        relation_dict = _construct_kg(kg_np)

        return relation_dict

    def _get_test_data(self, inter_test_data: np.array) -> tuple:
        """Generating interaction testing data by visiting inter_test_data.
        """
        inter_test_e = np.zeros(shape=inter_test_data.size)
        inter_test_neg = np.zeros(shape=inter_test_data.size)

        for idx, inter in enumerate(inter_test_data):
            e_id, neg_id = inter[0], inter[1]
            inter_test_e[idx] = e_id
            inter_test_neg[idx] = neg_id
        
        self.n_batch_test = self.n_test_inter // self.batch_size_test
        if self.n_batch_test == 0:
            self.n_batch_test = 1
        elif self.n_test_inter % self.n_batch_test:
            self.n_batch_test += 1

        return inter_test_e, inter_test_neg

    def _get_val_data(self, inter_val_data: np.array) -> tuple:
        """Generating interaction validating data by visiting inter_val_data.
        """
        inter_val_e = np.zeros(shape=inter_val_data.size)
        inter_val_neg = np.zeros(shape=inter_val_data.size)
        
        for idx, inter in enumerate(inter_val_data):
            e_id, neg_id = inter[0], inter[1]
            inter_val_e[idx] = e_id
            inter_val_neg[idx] = neg_id
        
        self.n_batch_val = self.n_val_inter // self.batch_size_val
        if  self.n_batch_val == 0:
            self.n_batch_val = 1
        elif self.n_val_inter % self.n_batch_val:
            self.n_batch_val += 1

        return inter_val_e, inter_val_neg

    def _load_ratings(self) -> tuple:
        """Loading system entity interaction data.
        """
        inter_mat = list()
        inter_dict = dict()
        
        # note: interaction data might be in different inter2id files        
        for inter_file in self.inter_file:            
            f = open(inter_file, 'r')
            lines = f.readlines()
            for l in lines:
                tmp = l.strip()
                inters = [int(i) for i in tmp.split(' ')]

                e_id, neg_ids = inters[0], inters[1:]
                # select UNIQUE negative ids
                neg_ids = list(set(neg_ids))

                if e_id not in inter_dict:
                    # situation: e_id first appears
                    for neg_id in neg_ids:
                        inter_mat.append([e_id, neg_id])
                    if len(neg_ids) > 0:
                        inter_dict[e_id] = neg_ids     
                else:
                    # situation: e_id has appeared in previous behaviors
                    exist_neg_ids = inter_dict[e_id]
                    diff_neg_ids = list(set(neg_ids) - set(exist_neg_ids))
                    exist_neg_ids.extend(diff_neg_ids)
                    for neg_id in diff_neg_ids:
                        inter_mat.append([e_id, neg_id])
            f.close()

        inter_data = np.array(inter_mat)
        self.n_inter = len(inter_data)

        return inter_data, inter_dict

    def _generate_train_inter_batch(self) -> tuple:
        """Generating training batch of system interaction.
        """
        def sample_neg_item_for_e(neg_items):
            n_neg_items = len(neg_items)
            neg_id = np.random.randint(low=0, high=n_neg_items)
            neg_i_id = neg_items[neg_id]

            return neg_i_id

        def sample_pos_item_for_e(neg_items, rate):
            items = []
            while len(items) != rate:
                pos_i_id = np.random.randint(low=0, high=self.n_entity)
                if pos_i_id not in neg_items and pos_i_id not in items:
                    items.append(pos_i_id)

            return items

        if self.batch_size_gnn <= self.exist_entity_size:
            e_batch = np.array(rd.sample(self.exist_entity, self.batch_size_gnn))
        else:
            e_batch = np.array([rd.choice(self.exist_entity) for _ in range(self.batch_size_gnn)])

        neg_batch = np.zeros(shape=[self.batch_size_gnn])
        pos_batch = np.zeros(shape=[self.batch_size_gnn * self.inter_pos_rate])

        for idx, e in enumerate(e_batch):
            neg_items = self.inter_dict[e]

            neg_e = sample_neg_item_for_e(neg_items)
            neg_batch[idx] = neg_e

            pos_e_list = sample_pos_item_for_e(neg_items, self.inter_pos_rate)
            for pos_idx, pos_e in enumerate(pos_e_list):
                pos_batch[idx * self.inter_pos_rate + pos_idx] = pos_e

        return e_batch, pos_batch, neg_batch
