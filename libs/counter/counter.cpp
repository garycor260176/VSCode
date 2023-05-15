#include "counter.h"

Counter::Counter(int _pin1, int _pin2, boolean _revers_sensor){
	revers = _revers_sensor;
	cur_state.pin_1.pin = _pin1;
	cur_state.pin_2.pin = _pin2;

	cur_state.pin_1.virtual_pin = (cur_state.pin_1.pin == 0 ? 1 : cur_state.pin_1.pin == 0);
	cur_state.pin_2.virtual_pin = (cur_state.pin_2.pin == 0 ? 2 : cur_state.pin_2.pin == 0);
}
	
void Counter::begin(){
  if(cur_state.pin_1.pin != 0) pinMode(cur_state.pin_1.pin, INPUT);
  if(cur_state.pin_2.pin != 0) pinMode(cur_state.pin_2.pin, INPUT);
  cur_state.pin_1.revers = cur_state.pin_2.revers = revers;
  cur_state.dir = first = 0;
  cur_state.any_HIGH = false;
}

Counter_sensor Counter::Clear_time(Counter_sensor sensor){
  Counter_sensor ret = sensor;
  ret.t_down  = ret.t_up = 0;
  return ret;
}

Counter_sensor Counter::sensor_read( Counter_sensor sensor){
  Counter_sensor ret = sensor;

   uint32_t mil = millis( );

  if(sensor.virtual_pin != 0){
    ret.state = Read_State(sensor.virtual_pin);
    if(ret.state != sensor.state){
        if(ret.state == HIGH){
          ret.t_up = mil;
        } else {
          ret.t_down = mil;
        }
    }

  } else {
    if((mil - ret.ContactTime) > 15 ) { // debounce of sensor signal 
      ret.state = digitalRead(ret.pin);
      if(ret.revers) {
        ret.state = !ret.state;
      }

      ret.ContactTime = mil; 

      if(ret.state != sensor.state){
          if(ret.state == HIGH){
            ret.t_up = mil;
          } else {
            ret.t_down = mil;
          }
      }
    } 
  }

  return ret;	
}

Counter_state Counter::read_state(){
	Counter_state ret = cur_state;

	ret.pin_1 = sensor_read(ret.pin_1);
	ret.pin_2 = sensor_read(ret.pin_2);
	
	ret.dir = 0;

	if(cur_state.pin_1.state != ret.pin_1.state || cur_state.pin_2.state != ret.pin_2.state){
    if(ret.pin_1.state == HIGH && ret.pin_1.t_up > 0 && ret.pin_2.state == LOW && ret.pin_2.t_up == 0){
      first = 1;
    } else if(ret.pin_2.state  == HIGH && ret.pin_2.t_up > 0 && ret.pin_1.state == LOW && ret.pin_1.t_up == 0){
      first = 2;
    }

    if( ret.pin_1.state == LOW && ret.pin_2.state == LOW) {
      if(	first == 1 &&
          ret.pin_2.t_down > 0 &&
          ret.pin_2.t_down >= ret.pin_2.t_up && ret.pin_2.t_down >= ret.pin_1.t_down && ret.pin_2.t_down >= ret.pin_1.t_up &&
          ret.pin_1.t_down >= ret.pin_1.t_up && ret.pin_1.t_down >= ret.pin_2.t_up &&
          ret.pin_2.t_up >= ret.pin_1.t_up
        ) 
        {
          ret.dir = 1;
        }
        else if( first == 2 &&
                ret.pin_1.t_down > 0 &&
                ret.pin_1.t_down >= ret.pin_1.t_up && ret.pin_1.t_down >= ret.pin_2.t_down && ret.pin_1.t_down >= ret.pin_2.t_up &&
                ret.pin_2.t_down >= ret.pin_2.t_up && ret.pin_2.t_down >= ret.pin_1.t_up &&
                ret.pin_1.t_up >= ret.pin_2.t_up
              ) 
        {
          ret.dir = 2;
        }
        ret.pin_1 = Clear_time(ret.pin_1);
        ret.pin_2 = Clear_time(ret.pin_2);
        first = 0;
      }
	}

  if( ret.pin_1.state == LOW && ret.pin_2.state == LOW) {
    ret.pin_1 = Clear_time(ret.pin_1);
    ret.pin_2 = Clear_time(ret.pin_2);
  }

	if(ret.pin_1.state == HIGH || ret.pin_2.state == HIGH){
		ret.any_HIGH = true;
	}
	else {
		ret.any_HIGH = false;
	}

	return ret;
}

void Counter::loop(){
	Counter_state state = read_state( );
	
	if (state.dir != cur_state.dir && state.dir != 0) {
		Counter_dir(state.dir);
	}
	if (state.any_HIGH != cur_state.any_HIGH ) {
		Counter_any_HIGH( state.any_HIGH );
	}
	
	cur_state = state;
}