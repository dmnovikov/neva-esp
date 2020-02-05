#ifndef neva_h
#define neva_h

#include "Arduino.h"
#include <SoftwareSerial.h>

#define RECONNECT_PERIOD 20000

struct meter_data {
    unsigned int val=0;
    int flg_ok=0;
    ulong last_get=0;
};



#define DEBUG

// OBIS codes without . and *
  const char* VOLTAGE="0C0700FF()";
  const char* FREQ="0E0701FF()";
  const char* CURRENT="0B0700FF()";  

class Neva {

private:


  SoftwareSerial neva;
  unsigned int m_current=0;
  unsigned int m_power=0;
  unsigned int m_voltage=0;
  unsigned int m_freq=0;
  String m_obis_str;
  int m_handshake=0;
  ulong m_last_reconnect=0;
  int m_meter_ok=0;

public:
  
  Neva(){};

  int virtual init(int _pin_RX, int _pin_TX) {
    
    neva.begin(9600, SWSERIAL_7E1,_pin_RX, _pin_TX, false, 64);
  };

  String read_line() {
    bool c = 1;
    char _b;
    String str_neva = "";
    bool pre_end_flg = 0;
    bool end_flg = 0;

    if(!neva.available()){
        return "";
    }

    while (c) {
      _b = neva.read();
      if (_b == 0x0D) {
        pre_end_flg = 1;
        // debug("PRE_END");
      }

      if (_b == 0x0A && pre_end_flg == 1) {
        end_flg = 1;
        c = 0;
        // debug("END");
      } else {
        // Serial.print(_b,HEX);
        // Serial.println("("+ String(char(_b)) + ")" );
        str_neva += char(_b);
      }
    }

    return str_neva;
  };

  char calc_bcc(String _s) {
    char _c;
    char _ret = 0;
    // Serial.println("SL="+String(_s.length()));
    for (int i = 0; i < _s.length(); i++) {
      _c = _s.charAt(i);
      // Serial.print("_c=");
      // Serial.println(_c, HEX);
      _ret = _ret ^ _c;
    }
    // Serial.print("BCC RET=");
    // Serial.println(_ret,HEX);
    return _ret;
  };

  String read_cmd_str() {
    bool c = 1;
    char _b;
    String str_neva = "";
    bool pre_end_flg = 0;
    bool end_flg = 0;
    int cmd_ok = 0;
    int i = 0;
    int cancel_char = 0;

    while (c) {
      _b = neva.read();

      // Serial.print(_b,HEX);
      // Serial.println("("+ String(char(_b)) + ")" );

      if (_b == 0x01) {
        cmd_ok = i;
        // debug("COMMAND detected");
        cancel_char = 1;
        // debug("111-M");
      }

      if (_b == 0x02) {
        cmd_ok = i;
        // debug("MESSAGE detected");
        cancel_char = 1;
        // debug("111-C");
      }

      if (pre_end_flg == 1) {
        c = 0;
        cancel_char = 1;
        // debug("END");
        // Serial.print("BCC=");
        // Serial.print(_b,HEX);
        // Serial.println();

        if (_b == calc_bcc(str_neva))
          debug("BCC OK");
        // debug("3333");

      } else if (_b == 0x03 || _b == 0x04) {
        pre_end_flg = 1;
        // debug("PRE-END");
        // debug("2222");
        cancel_char = 0;
      }

      if (cancel_char == 0) {
        str_neva += char(_b);
      }
      cancel_char = 0;
      i++;
    }

    return str_neva;
  };

  int read_ack_nak() {
    bool c = 1;
    char _b;
    String str_neva = "";
    bool pre_end_flg = 0;
    bool end_flg = 0;
    int cmd_ok = 0;
    int i = 0;
    int cancel_char = 0;

    while (c) {
      _b = neva.read();

      // Serial.print(_b,HEX);
      // Serial.println("("+ String(char(_b)) + ")" );

      if (_b == 0x06) {
        cmd_ok = i;
        debug("ASK OK");
        return 1;
        c = 0;
      }

      if (_b == 0x0F) {
        cmd_ok = i;
        debug("NAK OK");
        return 0;
        c = 0;
      }

      if (i > 3)
        return -1;

      i++;
    }

    return -1;
  };

  String create_cmd_str(String _cmd, String _data) {
    String _s = "";
    _s += char(0x01);
    _s += _cmd;
    _s += char(0x02);
    _s += _data;
    _s += char(0x03);
    char _bcc = calc_bcc(_s.substring(1));
    Serial.print("_bcc=");
    Serial.println(_bcc, HEX);
    _s += char(_bcc);

    // debug("S.len="+String(_s.length()));

    // for(int i=0;i<_s.length(); i++){
    //  Serial.print("_sndchar=");
    //  Serial.println(_s.charAt(i),HEX);
    //}

    // Serial.print("send_str="+_s);
    return _s;
  };

  String get_obis_str(String _data) {
    String _s_out = create_cmd_str("R1", _data);
    debug("OBISSTR","--->" + _s_out);
    neva.write(_s_out.c_str());

    delay_uart();
    String _s_in = read_cmd_str();
    if(_s_in=="") return "";
    debug("OBISSTR","<--- _s_in=" + _s_in);
    return _s_in;
  };

  int calc_obis_str(String _data){
    String _s_out = create_cmd_str("R1", _data);
    debug("OBISSTR","--->" + _s_out);
    neva.write(_s_out.c_str());

    delay_uart();
    m_obis_str = read_cmd_str();
    if(m_obis_str=="") return 0;
    debug("OBISSTR","<--- m_obis_str=" + m_obis_str);
    return 1;
  }

  int voltage_loop(){
      if(!m_handshake) return 0;

      if(calc_obis_str(VOLTAGE)){

          //m_obis_str -> int 

      }
      

  }

  int reconnect(){
      if(millis()- m_last_reconnect > RECONNECT_PERIOD){
          handshake_neva();
          m_last_reconnect=millis();
      }
  }




  int handshake_neva() {
    m_last_reconnect=millis();
    String str_init = "";
    String str_neva = "";

    // Ask hello (/?!\n\n)
    str_init = "/?!";
    str_init += char(0x0D);
    str_init += char(0x0A);

    debug("--->" + str_init);

    neva.write(str_init.c_str());

    delay_uart();

    String _s1 = read_line(); // get customer, name

    if(_s1=="") {
        debug("Error reading line");
        return 0;
    }

    debug("<--- s1=" + _s1);

    str_neva = "";
    str_init = "";
    // pre_end=0;

    // send ACK

    str_init += char(0x06); // ACK
    str_init += '0'; // v -- Управляющий символ: процедура протокола: '0' -
                     // нормальная; '1' - вторичная
    str_init += '5'; // 5 -9600
    str_init += '1'; // y -- Управляющий символ: '0' - считывание данных; '1' -
                     // режим программирования
    str_init += char(0x0D);
    str_init += char(0x0A);

    debug("Send ACK");

    neva.write(str_init.c_str());
    debug("---->" + str_init);
    delay_uart();
    // get password P0(00000000)
    String _s_in = read_cmd_str();
    debug("<--- _s_in=" + _s_in);

    // send password
    String _cmd = "P1";
    String _data = "(00000000)";
    String _s_out = create_cmd_str(_cmd, _data);
    debug("--->" + _s_out);
    neva.write(_s_out.c_str());

    delay_uart();

    // get ACK \ NAK

    if (read_ack_nak() == 1) {
      debug("<---ACK");
      m_handshake=1;
      return 1;
    }

    if (read_ack_nak() == 0) {
      debug("<---NAK");
      return 0;
    }

    if (read_ack_nak() == -1) {
      debug("<---ERROR");
      return -1;
    }
  };

protected:
  int debug(String _s) { 
    #ifdef DEBUG
       Serial.println(_s); 
    #endif
    
    };

  int debug(String _cs, String _s) { 
    #ifdef DEBUG
      Serial.println(_cs + ":" + _s); 
    #endif
    };

  int delay_uart() { delay(200); };
};

#endif