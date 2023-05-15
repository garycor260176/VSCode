#ifndef counter_h														//
#define counter_h														//

#include <Arduino.h>

struct Counter_sensor{
  uint32_t ContactTime;
  uint32_t t_up; 
  uint32_t t_down;
  int pin;
  int state;

  boolean revers;  

  int virtual_pin;
};

struct Counter_state{
	Counter_sensor pin_1;
	Counter_sensor pin_2;

	boolean any_HIGH;
	int dir;
};

void Counter_dir( int dir );
void Counter_any_HIGH( boolean anyHIGH);
int Read_State( int pin );

class Counter{
  public:
	Counter(int _pin1, int _pin2, boolean _revers_sensor = false);
	void begin( );
	void loop( );
	
  private:
	boolean revers;
	int first;
	Counter_state cur_state;
	
	Counter_sensor Clear_time(Counter_sensor sensor);
	Counter_sensor sensor_read( Counter_sensor sensor);
	Counter_state read_state();
};

#endif																			  //