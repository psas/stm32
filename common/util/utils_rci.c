#include "string.h"
#include "utils_general.h"
#include "rci.h"

/* GIT_COMMIT_VERSION is inserted by the build system, generated in
 * common/psas.mk
 */
static void version(struct RCICmdData * cmd UNUSED, struct RCIRetData * ret, void * user UNUSED){
#ifndef GIT_COMMIT_VERSION
#define GIT_COMMIT_VERSION "Unknown"
#endif
	strcpy(ret->data, GIT_COMMIT_VERSION);
	ret->len=sizeof(GIT_COMMIT_VERSION);
}
const struct RCICommand RCI_CMD_VERS = {
	.name="#VERS",
	.function=version,
	.user=NULL
};
