#define X_SIZE 80
#define Y_SIZE 80
#define Y_SIZEB (Y_SIZE/8)

const uint8_t img[800] = {255,255,255,255,255,255,255,255,255,255,255,224,31,255,255,255,255,255,255,255,255,208,67,255,255,255,255,255,255,255,255,208,197,255,255,255,255,255,255,255,255,132,200,63,255,255,255,255,255,255,255,140,9,79,255,255,255,255,255,255,255,144,0,0,255,255,255,255,255,255,255,208,0,12,159,255,255,255,255,255,255,224,0,192,0,255,255,255,255,255,255,224,14,1,0,63,255,255,255,255,255,224,14,8,0,15,255,255,255,255,255,224,14,0,0,7,255,255,255,255,255,240,12,0,0,3,137,15,255,255,255,240,0,1,227,226,192,1,255,255,255,248,112,121,247,224,36,4,255,255,255,252,176,125,247,224,3,128,255,255,255,252,0,61,247,225,19,192,31,255,255,254,3,63,239,224,119,220,47,255,255,255,195,61,239,224,119,190,7,255,255,255,0,29,239,194,231,62,43,255,255,255,132,15,239,194,230,125,1,255,255,255,198,29,239,194,236,103,133,255,255,255,236,61,239,194,224,31,128,255,255,255,252,62,223,194,206,63,146,255,255,255,248,63,95,194,158,124,0,255,255,255,252,31,159,192,60,227,201,255,255,255,255,63,223,192,124,143,235,255,255,255,255,247,239,193,248,63,1,255,255,255,255,211,239,193,248,252,99,255,255,255,255,225,247,195,243,241,231,255,255,255,255,249,251,195,231,195,239,255,255,255,255,252,123,195,207,31,255,255,255,255,255,254,61,195,158,63,255,255,255,255,255,255,142,131,56,247,255,255,255,255,255,255,199,2,103,191,255,255,255,255,255,255,243,0,158,127,255,255,255,255,255,255,249,0,63,255,255,255,255,255,255,255,254,0,255,255,255,255,255,255,255,255,240,0,15,255,255,255,0,0,0,0,15,135,255,255,0,0,0,0,0,0,5,255,128,0,0,0,0,0,0,0,3,255,128,0,0,0,0,0,0,0,4,255,198,0,0,0,0,0,0,0,25,255,49,128,0,0,0,0,0,0,113,121,24,32,0,0,0,0,0,0,227,120,135,8,0,0,0,0,0,3,194,120,65,193,0,0,0,0,0,7,4,124,48,224,0,0,0,0,0,14,8,60,56,60,16,0,0,0,0,28,0,60,28,14,24,0,0,0,0,88,16,60,15,3,156,0,0,0,0,16,32,62,7,192,246,0,0,0,1,0,96,127,134,112,21,0,0,0,3,192,64,123,195,28,54,0,0,0,7,128,160,122,97,3,255,0,0,0,7,129,32,122,49,128,109,0,0,0,7,198,32,123,61,224,127,0,0,0,63,196,32,121,23,152,122,0,0,0,115,224,16,125,17,132,250,0,0,0,255,196,16,61,16,129,212,0,0,1,125,132,16,63,24,193,120,0,0,1,125,128,16,63,8,99,216,0,0,3,253,128,16,63,136,63,224,0,0,7,31,132,16,62,252,61,64,0,0,6,143,132,24,63,255,255,0,0,0,15,159,142,24,63,91,251,0,0,0,15,255,254,62,253,237,252,0,0,0,31,243,255,255,248,103,224,0,0,0,31,225,255,255,248,0,0,0,0,0,63,241,247,255,224,0,0,0,0,0,31,243,126,255,128,0,0,0,0,0,31,255,63,120,0,0,0,0,0,0,79,255,247,64,0,0,0,0,0,0,95,255,188,0,0,0,0,0,0,0,119,245,176,0,0,0,0,0,0,0,114,119,128,0,0,0,0,0,0,0,47,126,0,0,0,0,0,0,0,0,79,184,0,0,0,0,0,0,0,0,31,224,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,};