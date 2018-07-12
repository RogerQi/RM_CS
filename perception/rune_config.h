#ifndef RUNE_CONFIG_H_
#define RUNE_CONFIG_H_

#define RUNE_PROTOTXT_PATH "./model/lenet.prototxt"
#define RUNE_MODEL_PATH    "./model/mnist_iter_75000.caffemodel"

#define W_CTR_LOW       64
#define W_CTR_HIGH      192
#define H_CTR_LOW       32
#define H_CTR_HIGH      96
#define HW_MIN_RATIO    0.3
#define HW_MAX_RATIO    0.75

#define BATCH_SIZE      15
#define CROP_SIZE       32
#define DIGIT_SIZE      28

#define DISTILL_RED_TH  70
#define MIN_RED_DIG_AREA 50

#endif
