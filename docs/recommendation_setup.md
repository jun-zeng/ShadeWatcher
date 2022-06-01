# Recommendation Setup
The setup has been tested using Python 3.6.5. The required packages are as follow:

1. Install Python virtualenv (optional)
```bash
pip3 install virtualenvwrapper
(better install with python3.6: python3.6 -m pip install virtualenvwrapper)
echo export WORKON_HOME=$HOME/.virtualenvs >> ~/.bashrc
(optional) echo export VIRTUALENVWRAPPER_PYTHON=/usr/bin/python3.6 >> ~/.bashrc
echo source /usr/local/bin/virtualenvwrapper.sh >> ~/.bashrc
(if virtualenvwrapper.sh cannot be found under /usr/local/bin: try echo source ~/.local/bin/virtualenvwrapper.sh >> ~/.bashrc)
source ~/.bashrc
mkvirtualenv shadewatcher -p python3.6
workon shadewatcher
```
2. tensorflow
```bash
(shadewatcher) pip install tensorflow_gpu==1.14
(shadewatcher) python -c "import tensorflow as tf;print(tf.reduce_sum(tf.random.normal([1000, 1000])))"
```
3. numpy
```bash
(shadewatcher) pip install numpy==1.19.2
```
4. scipy
```bash
(shadewatcher) pip install scipy==1.5.3
```
5. sklearn
```bash
(shadewatcher) pip install scikit-learn==0.19.0
```
6. tqdm
```bash
(shadewatcher) pip install tqdm
```
7. colorlog
```bash
(shadewatcher) pip install colorlog
```