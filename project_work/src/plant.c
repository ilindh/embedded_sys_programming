/**
 * @file plant.c
 * @brief Contains all the code related to the converter model
 */

#include "plant.h"

// Discretized model copied from assignment instruction sheet:
static const float A_matrix[6][6] = 	{{0.9652, -0.0172, 0.0057, -0.0058, 0.0052, -0.0251},
										{0.7732, 0.1252, 0.2315, 0.07, 0.1282, 0.7754},
										{0.8278, -0.7522, -0.0956, 0.3299, -0.4855, 0.3915},
										{0.9948, 0.2655, -0.3848, 0.4212, 0.3927, 0.2899},
										{0.7648, -0.4165, -0.4855, -0.3366, -0.0986, 0.7281},
										{1.1056, 0.7587, 0.1179, 0.0748, -0.2192, 0.1491}};

static const float B_matrix[6][1] = 	{{0.0471},
										{0.0377},
										{0.0404},
										{0.0485},
										{0.0375},
										{0.0539}};

static const float C_matrix[1][6] = 	{{0,0,0,0,0,1}};
static float current_state[6][1] = 		{{0},{0},{0},{0},{0},{0}};

// float i1 = 0;
// float u1 = 0;
// float i2 = 0;
// float u2 = 0;
// float i3 = 0;
// float u3 = 0;

float u_out = 0;

// Calculate plant response to input:
float plant_response(float u_in){

	static float current_state[6][1];

	// current_state = A_matrix*current_state + B_matrix*u_in;
	// u_out = current_state[5];

	return u_out;
}
