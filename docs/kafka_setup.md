# Kafka
Kafka is a distributed event streaming platform that lets you read, write, store, and process events (also called records or messages in the documentation) across many machines.

## Setup 
### Download Kafka
```bash
wget https://downloads.apache.org/kafka/2.8.0/kafka_2.13-2.8.0.tgz
```

### Start Zookeeper server
Depend on Java 8+; Better run Zookeeper through tmux
```bash
# Start the ZooKeeper service
# Note: Soon, ZooKeeper will no longer be required by Apache Kafka.
$ bin/zookeeper-server-start.sh config/zookeeper.properties
```

### Start Kafka broker
Better start the Kafka broker service through tmux
```bash
# Start the Kafka broker service (Please use our server.properties in ShadeWatcher/config)
bin/kafka-server-start.sh config/server.properties
```

### Create a topic
```bash
# Create the Kafka topic named quickstart-events 
bin/kafka-topics.sh --create --bootstrap-server localhost:9092 --topic quickstart-events
```

### Delete a topic
```bash
# List all Kafka topics (Note: set delete.topic.enable in config/server.properties)
bin/kafka-topics.sh --zookeeper localhost:2181 --list
bin/kafka-topics.sh --zookeeper localhost:2181 --delete --topic quickstart-events 
```

### Create a producer
```bash
bin/kafka-console-producer.sh --bootstrap-server localhost:9092 --topic quickstart-events
```

### Create a consumer (Optional)
```bash
bin/kafka-console-consumer.sh --topic quickstart-events --from-beginning --bootstrap-server localhost:9092
```

### Reset offset for a consumer (Optional)
```bash
bin/kafka-consumer-groups.sh --bootstrap-server localhost:9092 --delete-offsets --group 1 --topic quickstart-events
```

### Config Auditbeat
```
# Configure what output to use when sending the data collected by the beat.

# output.file:
#   path: "/var/log/auditbeat"
#   filename: auditbeat
#   rotate_every_kb: 8388608
#   number_of_files: 10
#   permissions: 0600
#   codec.json:
#   pretty: false

output.kafka:
  # initial brokers for reading cluster metadata
  hosts: ["localhost:9092"]

  # message topic selection + partitioning
  topic: 'quickstart-events'
  partition.round_robin:
    reachable_only: false

  required_acks: 1
  # compression: gzip
  max_message_bytes: 1000000
```
