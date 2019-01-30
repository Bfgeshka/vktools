#ifndef CONFIG_H_
#define CONFIG_H_

/*
 * How to get access token:
 * https://vk.com/dev/auth_mobile
 * recommended permissions: audio,video,docs,photos
 *
 * Use 'offline' permission if you want permanent token. Be careful with it.
 * You can add your permanent token after '=' in CONST_TOKEN string.
 */
#define PERMISSIONS "video,docs,photos,offline"
#define TOKEN_HEAD "&access_token="
#define CONST_TOKEN "&access_token="

#endif
