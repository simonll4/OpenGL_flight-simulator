#include "core/Application.h"
#include "crash/CrashHandler.h"

int main() {
	crash::installCrashHandler();
	core::Application app;
	return app.run();
}
