#include "string.h"
#include "utils_general.h"
#include "rnet_cmd_interp.h"

/* GIT_COMMIT_VERSION is inserted by the build system, generated in
 * common/psas.mk
 */
static void version(struct RCICmdData * rci_data, void * user_data UNUSED){
#ifndef GIT_COMMIT_VERSION
#define GIT_COMMIT_VERSION "Unknown"
#endif
	strcpy(rci_data->return_data, GIT_COMMIT_VERSION);
	rci_data->return_len=sizeof(GIT_COMMIT_VERSION);
}
const struct RCICommand RCI_CMD_VERS = {
	.name="#VERS",
	.function=version,
	.user_data=NULL
};
