#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  yield();
  for(int i = 0; i < len; i++) putch(((char*)buf)[i]);
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  yield();
  bool keydown = io_read(AM_INPUT_KEYBRD).keydown;
  int keycode = io_read(AM_INPUT_KEYBRD).keycode;
  if(keycode == AM_KEY_NONE){
    *((char*)buf) = '\0';
    return 0;
  }
  int ret = snprintf((char*)buf, len, "%s ==> %s", keyname[keycode], keydown ? "keydown" : "keyup");
  printf("%s\n", buf);
  return ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
