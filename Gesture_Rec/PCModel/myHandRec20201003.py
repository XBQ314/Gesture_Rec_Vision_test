from keras import models
from keras import layers
from keras.utils import to_categorical
import numpy as np

if __name__ == "__main__":
    # 分别导入各个标签的训练集
    train_data_0 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\train\\amgnothing.txt", delimiter=",", dtype=int)
    train_data_1 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\train\\amgup.txt", delimiter=",", dtype=int)
    train_data_2 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\train\\amgdown.txt", delimiter=",", dtype=int)
    train_data_3 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\train\\amgleft.txt", delimiter=",", dtype=int)
    train_data_4 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\train\\amgright.txt", delimiter=",", dtype=int)
    # 计算总样本数
    total_num_train = train_data_0.shape[0] + train_data_1.shape[0] + train_data_2.shape[0] + train_data_3.shape[0] + \
                      train_data_4.shape[0]
    # 把训练集放到一个数组里
    train_data = np.concatenate((train_data_0, train_data_1, train_data_2, train_data_3, train_data_4), axis=0)
    train_data = train_data.astype('float32') / 255
    # 生成对应形状的标签
    train_labels = np.zeros(total_num_train)
    # 给标签赋值
    train_labels[:train_data_0.shape[0]] = 0
    train_labels[train_data_0.shape[0]:train_data_0.shape[0] + train_data_1.shape[0]] = 1
    train_labels[
    train_data_0.shape[0] + train_data_1.shape[0]:train_data_0.shape[0] + train_data_1.shape[0] + train_data_2.shape[
        0]] = 2
    train_labels[
    train_data_0.shape[0] + train_data_1.shape[0] + train_data_2.shape[0]:train_data_0.shape[0] + train_data_1.shape[
        0] + train_data_2.shape[0] + train_data_3.shape[0]] = 3
    train_labels[
    train_data_0.shape[0] + train_data_1.shape[0] + train_data_2.shape[0] + train_data_3.shape[0]:total_num_train] = 4
    # 把标签转化为onehot
    train_labels = to_categorical(train_labels)

    # 分别导入各个标签的训练集
    test_data_0 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\test\\amgnothing.txt", delimiter=",", dtype=int)
    test_data_1 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\test\\amgup.txt", delimiter=",", dtype=int)
    test_data_2 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\test\\amgdown.txt", delimiter=",", dtype=int)
    test_data_3 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\test\\amgleft.txt", delimiter=",", dtype=int)
    test_data_4 = np.genfromtxt("D:\\Py Project\\NN\\Keras\\DataSet\\test\\amgright.txt", delimiter=",", dtype=int)
    # 计算总样本数
    total_num_test = test_data_0.shape[0] + test_data_1.shape[0] + test_data_2.shape[0] + test_data_3.shape[0] + \
                     test_data_4.shape[0]
    # 把训练集放到一个数组里
    test_data = np.concatenate((test_data_0, test_data_1, test_data_2, test_data_3, test_data_4), axis=0)
    test_data = test_data.astype('float32') / 255
    # 生成对应形状的标签
    test_labels = np.zeros(total_num_test)
    # 给标签赋值
    test_labels[:test_data_0.shape[0]] = 0
    test_labels[test_data_0.shape[0]:test_data_0.shape[0] + test_data_1.shape[0]] = 1
    test_labels[
    test_data_0.shape[0] + test_data_1.shape[0]:test_data_0.shape[0] + test_data_1.shape[0] + test_data_2.shape[
        0]] = 2
    test_labels[
    test_data_0.shape[0] + test_data_1.shape[0] + test_data_2.shape[0]:test_data_0.shape[0] + test_data_1.shape[
        0] + test_data_2.shape[0] + test_data_3.shape[0]] = 3
    test_labels[
    test_data_0.shape[0] + test_data_1.shape[0] + test_data_2.shape[0] + test_data_3.shape[0]:total_num_test] = 4
    # 把标签转化为onehot
    test_labels = to_categorical(test_labels)

    network = models.Sequential()
    network.add(layers.Dense(32, activation='relu', input_shape=(8 * 8,)))
    # network.add(layers.Dropout(0.5))
    network.add(layers.Dense(16, activation='relu'))
    # network.add(layers.Dropout(0.5))
    network.add(layers.Dense(5, activation='softmax'))
    network.compile(optimizer='rmsprop', loss='categorical_crossentropy', metrics=['accuracy'])

    network.fit(train_data, train_labels, epochs=8, batch_size=256)
    # network.save("myHandRec20201015.h5")

    test_loss, test_acc = network.evaluate(test_data, test_labels)
    print("test_loss:", test_loss)
    print("test_acc:", test_acc)
