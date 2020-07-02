## Secure Machine Learning with TEE

### About this project
This is the final project of class "SJTU Computer Architecture, 2019". 

In this project, we build a TEE module (consisting of TA, CA and other related codes) through ARM TrustZone (with OP-TEE) to guarantee the security and privacy of the inference process of some simple machine learning models. The three machine learning algorithms chosen are Logistic Regression, KNN and a 1-layer simple neural network.

We have deployed this TEE module on an ARM based device Raspberry Pi Model 3B, which shows its feasibility. 

### Environment 
- We compile OP-TEE and our TEE module on the following environment
  + Ubuntu 18.04, kernel 5.0.0-37-generic
  + gcc version 7.4.0+
  + repo version v1.13.8+
  + minicom version 2.7.1+
  + OP-TEE version 3.4.0+
- We need an ARM-based device to deploy our TEE module
  + Raspberry Pi Model 3B has been tested OK, while other (virtual) devices such as QEMU may also be OK, see [Platform Information](https://optee.readthedocs.io/en/latest/general/platforms.html) for more details.
  + You may need a USB to UART module to connect the PC and the arm device, we use CP2102.
  
### How to use it
1. Follow the official guide provided by OP-TEE, see [their website](https://optee.readthedocs.io/en/latest/building/index.html) for more details. If you use Raspberry Pi like us, see [this website](https://optee.readthedocs.io/en/latest/building/devices/rpi3.html). Stop before flashing the device.
2. Copy this whole folder into `$PROJECT_DIR/optee_examples`, just like other examples including `random` and `hello_world`.
3. Go into `$PROJECT_DIR/build`, and run the following codes
```
$ make all
$ make update_rootfs
```
4. Flash the OP-TEE OS and the rootfs to the SD-card, referring to [this website](https://optee.readthedocs.io/en/latest/building/devices/rpi3.html).
5. Put the SD-card back into the Raspberry Pi Model 3B. Plug in the UART cable and attach to the UART (with modules like CP2012).
6. Open minicom on your Linux PC with the following command, config it to detect serial device `/dev/ttyUSB0`, and set 'Hardware Control Flow' to be 'No'. 
```
$ sudo minicom -s
```
7. Switch on the Raspberry Pi Model 3B (or other devices you choose), you will see shell on minicom.
8. Run the following command to test this TEE module
```
$ /usr/bin/tee_secure_ml
```

### Expected console feedback
```
$ /usr/bin/tee_secure_ml 
Prepare D/TC:? 0 tee_ta_init_pseudo_ta_session:276 Lookup pseudo TA ffff50bb-144
session D/TC:? 0 load_elf:827 Lookup user TA ELF ffff50bb-1437-4fbf-8785-8d3580)
with the TA                                                                  
D/TC:? 0 load_elf:827 Lookup user TA ELF ffff50bb-1437-4fbf-8785-8d3580c34994 ()
D/TC:? 0 load_elf_from_store:795 ELF load address 0x40005000                 
D/TC:? 0 tee_ta_init_user_ta_session:1017 Processing relocations in ffff50bb-144
                                                                             
Test TEE logistic regression                                                 
- Create and load LR weight in the TA secure storage                         
The weight for LR is: 0.2 0.5 -0.25 -0.4                                     
- Find LR weight in TEE and do inference                                     
The inference instance for LR is: 0.5 0.2 0.4 0.25                           
Logistic Regression Result: 0.5                                              
- Delete LR weight in TEE        

Test TEE KNN.                                                             
- Create and load weight in the TA secure storage                         
The weight for KNN is: -0.5 -0.4 -0.3 -0.2 -0.1 0 0.1 0.2 0.3 0.4         
- Find KNN weight in TEE and do inference                                 
The inference instance for KNN is: 0.17                                
KNN Result: 0.2                                                        
- Delete KNN weight in TEE                                             
                                                                       
Test TEE neural network.
- Create and load neural network weight in the TA secure storage
The weight for neural network is: 
-0.5 -0.4 0.3 -0.2 0.5 0 0.1 0.2 -0.3 0.5 0.2 -0.2 -0.3 0.4 0.5 
- Find neural network weight in TEE and do inference
The inference instance for neural network is: 0.17 0.13 -0.1 -0.4 
Neural Network Result: 0.313774, 0.383244, 0.302982
- Delete neural network weight in TEE

We're doD/TC:? 0 tee_ta_close_session:380 tee_ta_close_session(0x10161780)
ne, closD/TC:? 0 tee_ta_close_session:399 Destroy session
e and reD/TC:? 0 tee_ta_close_session:425 Destroy TA ctx
lease TEE resources
```

### Contributors
- [Jilai Zheng](https://github.com/zhengjilai) 

### References
- [OPTEE official documentation](https://optee.readthedocs.io/en/latest/index.html)

- [OPTEE OS Project on github](https://github.com/OP-TEE/optee_os)

- [OPTEE Examples](https://github.com/linaro-swg/optee_examples)

  
