#ifndef __CLIENT_H__
#define __CLIENT_H__

/******************************************************************************
 Local Constant definitions
 *****************************************************************************/

#define PIR_PIN				(17)
#define PIR_STATUS_FILE     ("/sys/class/gpio/gpio17/value")

// #define SERVER_IP           ("192.168.1.25")
#define SERVER_IP           ("192.168.102.242")
#define SERVER_PORT 		(9001)
#define BUFFER_SIZE 		(10000)
#define CONNECTION_TIMEOUT	(180*1000)
#define CL_SUCCESS			(0)
#define CL_FAILED			(-1)

#define REQ_CMD			    ("HANDLE")
#define RESP_CMD		    ("OK")
#define RESP_MOTION_LABEL	("MOTION")
#define RESP_CODE_OK		(200)
#define RESP_CODE_FAILE 	(400)
#define RESP_CODE_SV_ERROR	(500)

/******************************************************************************
 Local Data type definitions
 *****************************************************************************/
typedef struct {
	int numImgs; 
} capture_imgs_t;

/******************************************************************************
 Function definitions
 *****************************************************************************/

int client_init(int *pSocket);
int client_deinit(int *pSocket);
int recv_cmd(char* buff);
void send_cmd(int socketFd, char* cmd, int param);
#endif