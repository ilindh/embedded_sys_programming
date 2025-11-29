/**
 * @file plant.c
 * @brief Contains all the code related to the converter model
 */

#include "plant.h"
#include "arm_math.h"

// Discretized model copied from assignment instruction sheet:
static const float32_t A_matrix[6][6] = {{0.9652, -0.0172, 0.0057, -0.0058, 0.0052, -0.0251},
										{0.7732, 0.1252, 0.2315, 0.07, 0.1282, 0.7754},
										{0.8278, -0.7522, -0.0956, 0.3299, -0.4855, 0.3915},
										{0.9948, 0.2655, -0.3848, 0.4212, 0.3927, 0.2899},
										{0.7648, -0.4165, -0.4855, -0.3366, -0.0986, 0.7281},
										{1.1056, 0.7587, 0.1179, 0.0748, -0.2192, 0.1491}};

static const float32_t B_matrix[6][1] = {{0.0471},
										{0.0377},
										{0.0404},
										{0.0485},
										{0.0375},
										{0.0539}};

static const float32_t C_matrix[1][6] = 	{{0,0,0,0,0,1}};

// This was changed from [6][1] to [6] because the [1] seemed redundant and produced an error
static float32_t current_state[6] = 		{0,0,0,0,0,0};


// Idk what these were for
// float i1 = 0;
// float u1 = 0;
// float i2 = 0;
// float u2 = 0;
// float i3 = 0;
// float u3 = 0;

/// @brief Calculates the plant response based on the input signal.
/// @param u_in The input signal to the plant.
/// @return The output response of the plant.
float32_t plant_response(float32_t u_in) {
	// Matrix multiplication implementation using CMSIS DSP library
	// Implementing this:
	// current_state = A_matrix*current_state + B_matrix*u_in;
	// u_out = current_state[5];

	// Initialize matrices
	arm_matrix_instance_f32 MatA;
	arm_mat_init_f32(&MatA, 6, 6, (float32_t *)A_matrix);

	// Temporary result matrices
	float32_t Ax_result[6]; // Result of A*current_state
	float32_t Bu_result[6]; // Result of B*u_in
	
	// static float current_state[6][1]; // I dont think this should be here?

	// First calculate Ax=A*current_state
	// ARM Matrix and vector multiplication function
	arm_mat_vec_mult_f32(&MatA, current_state, Ax_result);

	// Then calculate Bu=B*u_in
	// ARM scale function to multiply matrix B by scalar u_in
	arm_scale_f32((float32_t *)B_matrix, u_in, Bu_result, 6);

	// Finally calculate current_state = Ax + Bu
	// ARM vector addition function which gives us the current_state
	arm_add_f32(Ax_result, Bu_result, current_state, 6);
	
	// the output u_out
	float32_t u_out = current_state[5]; // defined locally

	return u_out;
}
