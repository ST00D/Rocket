#include <math.h>
#include <stdio.h>

// Defines the Square Wave Output Pin
#define OUT_PIN 4  // Audio is on pin 4
#define PTT_PIN 6  // PTT is on pin 6

#define _1200   1
#define _2400   0

#define _FLAG       0x7e
#define _CTRL_ID    0x03
#define _PID        0xf0
#define _DT_EXP     ','
#define _DT_STATUS  '>'
#define _DT_POS     '!'

#define _FIXPOS         1
#define _STATUS         2
#define _FIXPOS_STATUS  3

bool nada = _2400;

/*
 * SQUARE WAVE SIGNAL GENERATION
 */
const float baud_adj = 0.975;
const float adj_1200 = 1.0 * baud_adj;
const float adj_2400 = 1.0 * baud_adj;
unsigned int tc1200 = (unsigned int)(0.5 * adj_1200 * 1000000.0 / 1200.0);
unsigned int tc2400 = (unsigned int)(0.5 * adj_2400 * 1000000.0 / 2400.0);

/*
 * This strings will be used to generate AFSK signals, over and over again.
 */
const char *mycall = "KK7VPV";
char myssid = 1;

const char *dest = "APRS";

const char *digi = "WIDE2";
char digissid = 1;

const char *mystatus = "Hello World";

unsigned int tx_delay = 5000;

char bit_stuff = 0;
unsigned short crc=0xffff;

void set_nada_1200(void);
void set_nada_2400(void);
void set_nada(bool nada);

void send_char_NRZI(unsigned char in_byte, bool enBitStuff);
void send_string_len(const char *in_string, int len);

void calc_crc(bool in_bit);
void send_crc(void);

void send_packet(char packet_type);
void send_flag(unsigned char flag_len);
void send_header(void);
void send_payload(char type);

void set_io(void);
void print_code_version(void);
void print_debug(char type);

void set_nada_1200(void)
{
  digitalWrite(OUT_PIN, HIGH);
  delayMicroseconds(tc1200);
  digitalWrite(OUT_PIN, LOW);
  delayMicroseconds(tc1200);
}

void set_nada_2400(void)
{
  digitalWrite(OUT_PIN, HIGH);
  delayMicroseconds(tc2400);
  digitalWrite(OUT_PIN, LOW);
  delayMicroseconds(tc2400);
  
  digitalWrite(OUT_PIN, HIGH);
  delayMicroseconds(tc2400);
  digitalWrite(OUT_PIN, LOW);
  delayMicroseconds(tc2400);
}

void set_nada(bool nada)
{
  if(nada)
    set_nada_1200();
  else
    set_nada_2400();
}

void calc_crc(bool in_bit)
{
  unsigned short xor_in;
  
  xor_in = crc ^ in_bit;
  crc >>= 1;

  if(xor_in & 0x01)
    crc ^= 0x8408;
}

void send_crc(void)
{
  unsigned char crc_lo = crc ^ 0xff;
  unsigned char crc_hi = (crc >> 8) ^ 0xff;

  send_char_NRZI(crc_lo, HIGH);
  send_char_NRZI(crc_hi, HIGH);
}

void send_header(void)
{
  char temp;

  temp = strlen(dest);
  for(int j=0; j<temp; j++)
    send_char_NRZI(dest[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI('0' << 1, HIGH);

  temp = strlen(mycall);
  for(int j=0; j<temp; j++)
    send_char_NRZI(mycall[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI((myssid + '0') << 1, HIGH);

  temp = strlen(digi);
  for(int j=0; j<temp; j++)
    send_char_NRZI(digi[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI(((digissid + '0') << 1) + 1, HIGH);

  send_char_NRZI(_CTRL_ID, HIGH);
  send_char_NRZI(_PID, HIGH);
}

void send_payload(char type)
{
  send_char_NRZI(_DT_STATUS, HIGH);
  send_string_len(mystatus, strlen(mystatus));
}

void send_char_NRZI(unsigned char in_byte, bool enBitStuff)
{
  bool bits;
  
  for(int i = 0; i < 8; i++)
  {
    bits = in_byte & 0x01;

    calc_crc(bits);

    if(bits)
    {
      set_nada(nada);
      bit_stuff++;

      if((enBitStuff) && (bit_stuff == 5))
      {
        nada ^= 1;
        set_nada(nada);
        
        bit_stuff = 0;
      }
    }
    else
    {
      nada ^= 1;
      set_nada(nada);

      bit_stuff = 0;
    }

    in_byte >>= 1;
  }
}

void send_string_len(const char *in_string, int len)
{
  for(int j=0; j<len; j++)
    send_char_NRZI(in_string[j], HIGH);
}

void send_flag(unsigned char flag_len)
{
  for(int j=0; j<flag_len; j++)
    send_char_NRZI(_FLAG, LOW); 
}

void send_packet(char packet_type)
{
  print_debug(packet_type);

  digitalWrite(LED_BUILTIN, 1);
  digitalWrite(PTT_PIN, HIGH); // Activate PTT
  
  send_flag(100);
  crc = 0xffff;
  send_header();
  send_payload(packet_type);
  send_crc();
  send_flag(3);

  digitalWrite(LED_BUILTIN, 0);
  digitalWrite(PTT_PIN, LOW); // Deactivate PTT
}

void set_io(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(PTT_PIN, OUTPUT);

  Serial.begin(115200);
}

void print_code_version(void)
{
  Serial.println(" ");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  
  Serial.println("Random String Pulsed AFSK Generator - Started \n");
}

void print_debug(char type)
{
  int temp;
  
  temp = strlen(mycall);
  for(int j=0; j<temp; j++)
    Serial.print(mycall[j]);
  Serial.print('-');
  Serial.print(myssid, DEC);
  Serial.print('>');

  temp = strlen(dest);
  for(int j=0; j<temp; j++)
    Serial.print(dest[j]);
  Serial.print(',');

  temp = strlen(digi);
  for(int j=0; j<temp; j++)
    Serial.print(digi[j]);
  Serial.print('-');
  Serial.print(digissid, DEC);
  Serial.print(':');

  Serial.print(_DT_STATUS);
  Serial.print(mystatus);
  
  Serial.println(' ');
}

void setup()
{
  set_io();
  print_code_version();
}

void loop()
{
  send_packet(_STATUS);
  delay(tx_delay);
}