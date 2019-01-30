#ifndef CONFIG_H_
#define CONFIG_H_

/*
 * For generation of token getting link.
 * Create your own application, get its ID and insert it here.
 *
 * For creating your application visit https://vk.com/editapp?act=create.
 * Choose "standalone application" and give it a name you like.
 *
 * After go to "My apps": https://vk.com/apps?act=manage
 * Go to 'manage' on the right side.
 *
 * Choose 'settings', and there you can see your application id.
 * Copy it and replace value of APPLICATION_ID.
 */
#define APPLICATION_ID 5934008

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
