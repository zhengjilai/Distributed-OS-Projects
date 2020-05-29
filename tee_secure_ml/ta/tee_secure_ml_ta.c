/*
 * Copyright (c) 2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_secure_ml_ta.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

static TEE_Result secure_ml_delete_weight(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_Result res;
	char *obj_id;
	size_t obj_id_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	/*
	 * Check object exists and delete it
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_ACCESS_WRITE_META, /* we must be allowed to delete it */
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		TEE_Free(obj_id);
		return res;
	}

	TEE_CloseAndDeletePersistentObject1(object);
	TEE_Free(obj_id);

	return res;
}

static TEE_Result secure_ml_init_weight(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_NONE,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_Result res;
	char *obj_id;
	size_t obj_id_sz;
	char *data;
	size_t data_sz;
	uint32_t obj_data_flag;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	data = (char *)params[1].memref.buffer;
	data_sz = params[1].memref.size;

	/*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE |		/* we can later write into the object */
			TEE_DATA_FLAG_ACCESS_WRITE_META |	/* we can later destroy or rename the object */
			TEE_DATA_FLAG_OVERWRITE;		/* destroy existing object of same ID */

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					obj_data_flag,
					TEE_HANDLE_NULL,
					NULL, 0,		/* we may not fill it right now */
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
		TEE_Free(obj_id);
		return res;
	}

	res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else {
		TEE_CloseObject(object);
	}
	TEE_Free(obj_id);
	return res;
}

static void char_to_float(char *c, float *f){
  	char sBuf[4];
  	sBuf[0]=c[0];
  	sBuf[1]=c[1];
  	sBuf[2]=c[2];
  	sBuf[3]=c[3];
	float tmp = *((float*)(&sBuf));
	*f = tmp;
}

static void float_to_char(float *f, char *sBuf){
    char* temp;
    temp = (char*)(f);
    sBuf[0] = temp[0] ;
    sBuf[1] = temp[1];
    sBuf[2] = temp[2];
    sBuf[3] = temp[3]; 
}

static float exponential(float ind){

	float fenmu = 1;
	float fenzi = ind;
	float res = 1.0;
	for (int i = 1; i < 10; i ++){
		res += fenzi / fenmu;
		fenmu *= (i+1);
		fenzi *= ind;
	}
	return res;
}

static float absolute(float a, float b){
	return a > b ? a-b : b-a;
}

static TEE_Result secure_ml_inference(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types =
		TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_INPUT,
				TEE_PARAM_TYPE_MEMREF_OUTPUT,
				TEE_PARAM_TYPE_NONE);
	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	size_t read_bytes;
	char *obj_id;
	size_t obj_id_sz;
	char *inference_data;
	size_t inference_data_sz;
	char *inference_output;
	size_t inference_output_sz;
	char *read_raw_data;
	
	float result, tmp1, tmp2, tmp3;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	inference_data_sz = params[1].memref.size;
	inference_data = TEE_Malloc(inference_data_sz, 0);
	if (!inference_data_sz)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(inference_data, params[1].memref.buffer, inference_data_sz);

	inference_output = (char *)params[2].memref.buffer;
	inference_output_sz = params[2].memref.size;

	/*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		TEE_Free(obj_id);
		return res;
	}

	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
		goto exit;
	}

	// if (object_info.dataSize > data_sz) {
	// 	/*
	// 	 * Provided buffer is too short.
	// 	 * Return the expected size together with status "short buffer"
	// 	 */
	// 	params[2].memref.size = object_info.dataSize;
	// 	res = TEE_ERROR_SHORT_BUFFER;
	// 	goto exit;
	// }

	read_raw_data = TEE_Malloc(object_info.dataSize, 0);
	res = TEE_ReadObjectData(object, read_raw_data, object_info.dataSize,
				 &read_bytes);
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
		EMSG("TEE_ReadObjectData failed 0x%08x, read %u over %u",
				res, read_bytes, object_info.dataSize);
		goto exit;
	}

	if (obj_id[0] == '1') { // logistic regression
	    // check whether inference data has the same size with weights
	    if (inference_data_sz != read_bytes){
			params[2].memref.size = 4;
			res = TEE_ERROR_SHORT_BUFFER;
			goto exit;
		} else if (inference_output_sz < 4 ){
			params[2].memref.size = 0;
			res = TEE_ERROR_SHORT_BUFFER;
			goto exit;
		}
		result = 0.0;
		for (size_t i = 0; i < read_bytes; i = i + 4){
			char_to_float(read_raw_data + i, &tmp1);
			char_to_float(inference_data + i, &tmp2);
			result += tmp1 * tmp2;
		}
		result = 1.0 / (1.0 + exponential(-result) );
		float_to_char(&result, inference_output);
		params[2].memref.size = 4;

	} else if (obj_id[0] == '2') { // k-NN
	    if (inference_output_sz < 4 ){
			params[2].memref.size = 0;
			res = TEE_ERROR_SHORT_BUFFER;
			goto exit;
		}
		result = 0.0;
		char_to_float(inference_data, &tmp1);
		tmp2 = 10000.0; // MAXDISTANCE
		for (size_t i = 0; i < read_bytes; i = i + 4){
			char_to_float(read_raw_data + i, &tmp3);
			if ( absolute(tmp1, tmp3) < tmp2 ) {
				tmp2 = absolute(tmp1, tmp3);
				result = tmp3;
			}
		}
		float_to_char(&result, inference_output);
		params[2].memref.size = 4;

	} else if (obj_id[0] == '3') { // 3-class neural network
	    // check whether inference data has the same size with weights
		// kx + b
	    if ( (inference_data_sz * 3 + 12) != read_bytes){
			params[2].memref.size = 12;
			res = TEE_ERROR_SHORT_BUFFER;
			goto exit;
		} else if (inference_output_sz < 12){
			params[2].memref.size = 0;
			res = TEE_ERROR_SHORT_BUFFER;
			goto exit;
		}
		float result_3_class[3];
		for (size_t k = 0; k < 3; k++){
			result = 0.0;
			for (size_t i = k * (read_bytes / 3); i < (k+1) * (read_bytes / 3); i = i + 4){
				if (i < ((k+1) * read_bytes / 3 - 4) ){
					char_to_float(read_raw_data + i, &tmp1);
					char_to_float(inference_data + (i % (read_bytes / 3)), &tmp2);
					result += tmp1 * tmp2;
				} else {
					char_to_float(read_raw_data + i, &tmp1);
					result += tmp1;
				}
			}	
			result_3_class[k] = result;
		}

		result = exponential(result_3_class[0]) + 
				exponential(result_3_class[1]) + 
				exponential(result_3_class[2]);
				
		for (size_t k = 0; k < 3; k++){
			result_3_class[k] = exponential(result_3_class[k]) / result;
			float_to_char(result_3_class + k, inference_output + (4*k));
		}
		params[2].memref.size = 12;
	} else {
		params[2].memref.size = 4;
	}
	

exit:
	TEE_CloseObject(object);
	TEE_Free(obj_id);
	return res;
}

TEE_Result TA_CreateEntryPoint(void)
{
	/* Nothing to do */
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	/* Nothing to do */
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
				    TEE_Param __unused params[4],
				    void __unused **session)
{
	/* Nothing to do */
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *session)
{
	/* Nothing to do */
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *session,
				      uint32_t command,
				      uint32_t param_types,
				      TEE_Param params[4])
{
	switch (command) {
	case TA_TEE_SECURE_ML_CMD_INIT:
		return secure_ml_init_weight(param_types, params);
	case TA_TEE_SECURE_ML_CMD_INFERENCE:
		return secure_ml_inference(param_types, params);
	case TA_TEE_SECURE_ML_CMD_DELETE:
		return secure_ml_delete_weight(param_types, params);
	default:
		EMSG("Command ID 0x%x is not supported", command);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
