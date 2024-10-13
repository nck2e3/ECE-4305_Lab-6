#include "chu_init.h"
#include "gpio_cores.h"

void uart_transmit_speed(int speed) {
   uart.disp("current speed: ");
   uart.disp(speed);
   uart.disp("\n\r");
}

/**
 * Function to handle LED chasing
 * @param led_p pointer to led instance
 * @param sw_p pointer to switch instance
 */
void led_chase(GpoCore *led_p, GpiCore *sw_p) {
   int pos = 0;              // LED position (0 is rightmost, 15 is leftmost)
   int direction = -1;       // -1 for left, 1 for right
   int speed, prev_speed = 0;
   int sw_value;
   int min_delay = 100;      // Minimum delay for the fastest speed (adjust as needed)
   int max_delay = 1000;     // Maximum delay for the slowest speed (adjust as needed)

   while (1) {
      // Check switch 0 to initialize the position to rightmost (pos = 0)
      if (sw_p->read(0)) {
         pos = 0;  // Move to the rightmost LED (pos = 0)
      }

      // Read switches sw1-sw5 for speed
      sw_value = (sw_p->read() >> 1) & 0x1F;  // Get the value of switches sw1-sw5
      
      // Invert the speed so higher switch values mean faster speed
      if (sw_value == 0) {
         speed = max_delay;  // Set to the slowest if all switches are off
      } else {
         speed = max_delay - ((sw_value * (max_delay - min_delay)) / 0x1F);  // Map the switch value to a delay
      }

      // If speed changes, send a message via UART
      if (speed != prev_speed) {
         uart_transmit_speed(sw_value);
         prev_speed = speed;
      }

      // Light up the current LED
      led_p->write(1 << pos);

      // Adjust position and direction
      sleep_ms(speed);  // Delay based on speed (inverted)
      led_p->write(0);  // Turn off the LED before moving

      if (pos == 0) {
         direction = 1;  // Change direction to right (increment)
      } else if (pos == 15) {
         direction = -1; // Change direction to left (decrement)
      }

      pos += direction;  // Move LED in current direction
   }
}

int main() {
   // Instantiate switch and LED objects
   GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
   GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));

   // Start LED chase program
   led_chase(&led, &sw);

   return 0;
}
