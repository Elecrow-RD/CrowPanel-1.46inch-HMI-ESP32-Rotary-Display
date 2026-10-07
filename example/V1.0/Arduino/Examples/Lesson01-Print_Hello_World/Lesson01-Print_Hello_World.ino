/*-----------------------------------------------------------------
 * Serial hello-world demonstration.
 * The sketch establishes a serial connection once, then emits a
 * recognizable message at a fixed interval for timing verification.
 *----------------------------------------------------------------*/

/**
 * @brief Initialize the serial port and print the startup message.
 * @param None.
 * @return None.
 * @call setup() is called once after reset.
 */
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Hello World");
}

/**
 * @brief Print the status message periodically.
 * @param None.
 * @return None.
 * @call loop() runs repeatedly after setup() completes.
 */
void loop() {
  delay(1000);
  Serial.println("Hello World");
}
