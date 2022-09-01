// GLOBAL CONTROLS
#define DAC_UPDATE 8 // DAC update interval [us]
#define PERIOD 4000 // Period of control cycle [us]
#define TRAP_TIME 2500 // Time for trapping [us]
#define THRESHOLD 0.1 // Triggering threshold for ADC (i.e. trigger when ADC input crosses this threshold)
#define OP_DELAY 500 // Time delay between trapping field and OP field [us]
#define OP_TIME 1000 // Timelength of optical pumping [us]

// Set magnetic fields
#define B_TRAP 60.0 // [G]
#define B_OP 2.0 // [G]

// Compute output voltages
// Matsusada Supplies CC: 7.8 A/V
// TRINAT Coil parameters: 1.11 G/A
const double v_trap = B_TRAP / 1.11 / 7.8;
const double v_op = B_OP / 1.11 / 7.8;

// Common objects
IntervalTimer Timer;
int time = 0;

// Update DAC function
float out;
float top;
float bottom;
void updateDAC() {
	if (time < PERIOD) {
		
		// Trapping field
		out = cosPower(0, TRAP_TIME, v_trap);
		
		// Optical pumping field
		top = out + cosPower(TRAP_TIME + OP_DELAY, OP_TIME, v_op);
		bottom = out - cosPower(TRAP_TIME + OP_DELAY, OP_TIME, v_op);
		
		writeDAC1(top);
		writeDAC2(bottom);

		time += DAC_UPDATE;
	} else {
		// After completing one cycle end Timer
		time = 0;
        Timer.end();
	}
}


bool state = false;
// ADC interrupt
// Tells adc to start cycle when input goes above some threshold voltage
// I.e. trigger Quarto on rising edge of some signal
void ADC_interrupt() {
    float V = readADC1_from_ISR();
    if ((V > THRESHOLD) & !state) {
		// Voltage crossed threshold and we were not in on state before
		// so we start updating the DAC using an interrupt timer.
        Timer.begin(updateDAC, DAC_UPDATE);
        state = true;
    } else if ((V < THRESHOLD) & state) {
        state = false;
    }
}

// Waveforms
float cosPower(int start_time, int length, float amplitude) {
	float out;
	if ((time >= start_time) && (time - start_time < length)) {
		// The commented out code below produces some weird spikes in the output.
		// out = - amplitude * pow(cos(PI * (time - start_time) / length), 8) + amplitude;
		float x = cos(PI * (time - start_time) / length);
		out = - amplitude * x * x * x * x * x * x * x * x + amplitude;
	} else {
		out = 0;
	}
	return out;
}

void setup() {
	// ADC value gets read out every 1 us in ADC_interrupt function defined above
    configureADC(1, 1, 1, BIPOLAR_10V, ADC_interrupt);
}

void loop() {
}
