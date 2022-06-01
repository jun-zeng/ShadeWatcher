# Encoding Data Description

We provide a simple example to demonstrate the data structure for model training.

- `entity2id.txt`
    - Entity file.
    - Each line is a triplet(`hash_id`, `onehot_id`) for each entity in the knowledge graph, where `hash_id` and `onehot_id` represent the ID of such entity in knowledge graph and our training dataset, respectively.

- `inter2id_o.txt`
    - Interaction file.
    - Each line is a system entity with its interactive entities. (`entity_id` and `a list of interactive entity id`)

- `relation2id.txt`
    - Relation file.
    - Each line is a relation (i.e., system call) with its unique id in our training dataset.

- `train2id.txt`
    - Train file.
    - Each line is a triplet(`entity1_id`, `entity2_id`, `relation_id`) for one interaction in knowledge graph, where `entity1_id` and `entity2_id` represent the ID of such interaction, and `relation_id` represents the relation between `entity1_id` and `entity2_id`. 