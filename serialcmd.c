/*
 *  Simple code to connect to a serial tty, send over a command and print
 *  the output. To make it easier when you need a simple console command to
 *  control an arduino, for example, automacally detect its port and print
 *  the output to STDOUT.
 *
 *  Nino MegaDriver - nino@nino.com.br
 *  http://www.megadriver
 *  License terms: Do whatever you want with it:D
 *
 *  Arduino LOOP example with sync:
 *  loop()
 *   {
 *     (...)
 *     Serial.write(0x00); // The sync char that this code uses
 *     while(!Serial.available() ){ } // Wait for a command over serial
 *     String input = Serial.readStringUntil('\n'); // Read it into a string
 *     if(input.equals("hello")){ // Process the commands
 *        Serial.println("Hello!");
 *     }else{
 *        Serial.println("Be polite, say something!");
 *     }
 *     (...)
 *     Serial.write(0x00); // Send the sync char again to tell it's finished
 *  }
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <inttypes.h>

int serial_connected = -1;
int BOUDRATE = B115200;

// Some dafult ports to try connecting to
const char* serial_ports[6] = {
	"/dev/ttyUSB0",
	"/dev/ttyUSB1",
	"/dev/ttyUSB2",
	"/dev/ttyACM0",
	"/dev/ttyACM1",
	"/dev/ttyACM2",
};
int serial_port    = 0;
int serial_error   = 0;


void trigger_error(const char* m){
    serial_error = 1;
    printf("ERROR %s %s (%d)\n",m, strerror(errno), errno);
    exit(1);
}

void serial_connect(){
  int retry = serial_port;
  while(serial_connected == -1 && retry <6){
    serial_port = retry;
    serial_connected = open(serial_ports[retry], O_RDWR);
    retry++;
  }

  if(serial_connected == -1){
    serial_port = -1;
    printf("ERROR: arduino not found.\n");
    exit(0);
  }

  printf("Connected to \"%s\"\n", serial_ports[retry-1]);

  struct termios tty;
  int i = tcgetattr(serial_connected, &tty);
  if(i<0){ trigger_error("[1]:"); return;}
  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
  tty.c_lflag &= ~ICANON; // Disable Cannonical Mode
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  tty.c_cc[VMIN]  = 1; // Activate Blocking
  tty.c_cc[VTIME] = 0; // Activate Blocking
  int j = cfsetispeed(&tty, BOUDRATE);
  if(j<0){ trigger_error("[2]:"); return;}
  int k = cfsetospeed(&tty, BOUDRATE);
  if(k<0){ trigger_error("[3]:"); return;}
  int l = tcsetattr(serial_connected, TCSANOW, &tty);
  if(l<0) {trigger_error("[4]:"); return;}
}

// Reads the serial port untin it gets sync char (0x00), and echo
// all printable characters to the screen.
void sync(){
    int r;
    char rect[1];
    r = read(serial_connected, &rect, 1);
    while(rect[0] != 0x00 && r>0){
      if(rect[0] > 0x07 && rect[0] < 0x7f) printf("%s", rect);
      r = read(serial_connected, &rect, 1);
    }
    printf("\n");
}

// Print usage
void usage(char *fname){
  printf("SERIALCMD v1.0 - nino@nino.com.br\n");
  printf("Send a command over serial and print the output\n\n");
  printf("Usage:\n%s -p [port, default: auto-detect] -b [boundrate, default: 115200] -c [cmd]\n\n", fname);
  exit(0);
}

int main(int argc, char **argv) {
    int r=0, col=0, trail = 0;;
    char cmd[300];
    unsigned char c[1];
    char input_port[100];

    if (argc==1) {
        usage(argv[0]);
    }

    int option_index = 0, opt;
    static struct option loptions[] = {
	      {"help",       no_argument,        0, 'h'},
        {"port",       optional_argument,  0, 'p'},
        {"boundrate",  optional_argument,  0, 'b'},
        {"cmd",        required_argument,  0, 'c'},
        {NULL,         0,                  0,  0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "h:p:b:c:", loptions, &option_index);
        if (opt==-1) break;
        switch (opt) {
      	  case 'h':
      	      usage(argv[0]);
      	      break;
          case 'p':
              for(int i=0; i<6;i++) if(strcmp(optarg, serial_ports[i]) == 0) serial_port = i;
              break;
          case 'b':
              BOUDRATE = (long)strtol(optarg, NULL, 0);
              break;
      	  case 'c':
      	      serial_connect();
      	      sleep(2);
              printf("<< Waiting for sync...\n");
              sync();
              sleep(2);
              strcpy(cmd,optarg);
              printf(">> %s\n", cmd);
      	      r = write(serial_connected, &cmd, 20);
              sync();
              exit(0);
          	  break;
        }
    }
    return 0;
}
