#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libusb-1.0/libusb.h>

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT                0x01
#define HID_GET_IDLE                  0x02
#define HID_GET_PROTOCOL              0x03
#define HID_SET_REPORT                0x09
#define HID_SET_IDLE                  0x0A
#define HID_SET_PROTOCOL              0x0B
#define HID_REPORT_TYPE_INPUT         0x01
#define HID_REPORT_TYPE_OUTPUT        0x02
#define HID_REPORT_TYPE_FEATURE       0x03

#define CTRL_IN        LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE
#define CTRL_OUT    LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE

#define VID 0x0451
#define PID 0xe012

#define STREQ(s1, s2) strcmp(s1, s2) == 0
#define ERROR(string, action) \
    if (r < 0) { \
        fprintf(stderr, string); \
        fprintf(stderr, "%s\n", libusb_strerror(r)); \
        action; \
    }

#define ASSURE_ARGC_M(count) \
        if (argc != count) { \
            usage(argv[0]); \
            printf("(got %d)\n", argc); \
            return 1; \
        }

#define ASSURE_ARGC(count) \
        if (argc != count) { \
            usage(argv[0]); \
            return 1; \
        }

const static int PACKET_CTRL_LEN = 8;
const static int PACKET_INT_LEN = 8;
const static int INTERFACE = 0;
const static int EP1_IN = 0x81; /* endpoint 0x81 address for IN */
const static int EP1_OUT = 0x01; /* endpoint 1 address for OUT */
const static int TIMEOUT = 5000; /* timeout in ms */

void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s led | noled | blink | noblink | clear\n",
            prog);
    fprintf(stderr, "       %s pwm \e[4mwidth\e[m\n", prog);
    fprintf(stderr, "       %s page \e[4mCS\e[m \e[4mdata\e[m\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "       \e[4mwidth\e[m is an integer in the range of 1~16 (inclusive).\n");
    fprintf(stderr, "       \e[4mCS\e[m is a chip-set parameter.\n");
    fprintf(stderr, "       \e[4mdata\e[m is 32 unsigned chars represented by hexs and seperated by spaces.\n");
}

static struct libusb_device_handle *devh = NULL;

static int find_lvr_hidusb(void) {
    devh = libusb_open_device_with_vid_pid(NULL, VID, PID);
    return devh ? 1 : -1;
}

static int control_write(char *request, int size) {
    int r = libusb_control_transfer(devh, CTRL_OUT, HID_SET_REPORT,
            HID_REPORT_TYPE_OUTPUT << 8, 0, request, size, TIMEOUT);
    ERROR("Control Out error: ", return r);
    return 0;
}

static int control_read(char *answer, int size) {
    int r = libusb_control_transfer(devh, CTRL_IN, HID_GET_REPORT,
            HID_REPORT_TYPE_INPUT << 8, 0, answer, size, TIMEOUT);
    ERROR("Control In error: ", return r);
    return 0;
}

int main(int argc, char **argv) {
    int r = 1;

    if (argc != 2) {
        usage(argv[0]);
        printf("(got %d)\n", argc);
        return 1;
    }
    
    r = libusb_init(NULL);
    ERROR("Failed to initialise libusb: ", exit(1));

    r = find_lvr_hidusb();
    ERROR("Could not find/open LVR Generic HID device: ", goto out);

#ifdef LINUX
    libusb_detach_kernel_driver(devh, 0);
#endif

    r = libusb_set_configuration(devh, 1);
    ERROR("libusb_set_configuration error: ", goto out);

    r = libusb_claim_interface(devh, 0);
    ERROR("libusb_claim_interface error: ", goto out);

    char *data = argv[1];
    int len = strlen(data);
    unsigned short head = (unsigned short) len;
    unsigned char len_data[2];
    len_data[0] = head >> 8;
    len_data[1] = head;
    printf("data length: %d|%x\n", len, len);
    control_write(len_data, 2);
    while (len > 0) {
        control_write(data, len >= 8 ? 8 : len);
        len -= 8;
        data += 8;
    }

    //printf("\nAnswer:\n");
    //unsigned char buf[8] = {0};
    //control_read(buf, 2);
    //printf("Header: ");
    //for (int i = 0; i < 8; i ++) {
    //    printf("%02x ", buf[i]);
    //}
    //printf("\n");
    //unsigned short len_ans = buf[0] << 8 | buf[1];
    //int real_len_ans = (int) len_ans;
    //printf("Length: %d\n", real_len_ans);
    //char *answer = calloc((real_len_ans + 1), sizeof(char));
    //while (real_len_ans > 0) {
    //    int packet_len = real_len_ans >= 8 ? 8 : real_len_ans;
    //    control_read(buf, 8);
    //    for (int i = 0; i < 8; i ++) {
    //        printf("%02x ", buf[i]);
    //    }
    //    printf("\n");
    //    strncat(answer, buf, packet_len);
    //    real_len_ans -= 8;
    //}
    //printf("Data: %s\n", answer);

    //unsigned char buf[128] = {0};
    //control_read(buf, 128);
    //unsigned short len_ans = buf[0] << 256 + buf[1];
    //printf("Length: %d\n", len_ans);
    ////char *answer = calloc((len_ans + 1), sizeof(char));
    ////while (len_ans > 0) {
    ////    int packet_len = len_ans >= 8 ? 8 : len_ans;
    ////    control_read(buf, packet_len);
    ////    strncat(answer, buf, packet_len);
    ////    len_ans -= 8;
    ////}
    //printf("Data: %s\n", buf);
    //for (int i = 0; i < 24; i ++) {
    //    printf("%02x ", buf[i]);
    //}
    //printf("\n");

    libusb_release_interface(devh, 0);

out:
//    libusb_reset_device(devh);
    libusb_close(devh);
    libusb_exit(NULL);
    return r >= 0 ? r : -r;
}
