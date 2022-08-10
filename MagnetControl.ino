// GLOBAL CONTROLS
#define DAC_UPDATE 8 // DAC update interval [us]
#define PERIOD 4000 // Period of control cycle [us]
#define TRAP_TIME 2500 // Time for trapping [us]
#define THRESHOLD 0.1 // Triggering threshold for ADC
#define OP_DELAY 500 // Time delay between trapping field and OP field [us]

// Common objects
IntervalTimer Timer;
int time = 0;

// Update DAC function
float out;
float top;
float bottom;
void updateDAC() {
	if (time < PERIOD) {
		
		// Trapping field ~50 A
		out = cosPower(0, TRAP_TIME, 7, 8);
		
		// Optical pumping field
		top = out + cosPower(TRAP_TIME + OP_DELAY, 1000, 0.256, 8);
		bottom = out - cosPower(TRAP_TIME + OP_DELAY, 1000, 0.256, 8);
		
		writeDAC1(top);
		writeDAC2(bottom);

		time += DAC_UPDATE;
	} else {
		time = 0;
        Timer.end();
	}
}


bool state = false;
// ADC interrupt
// Tells adc to start cycle when input goes above some threshold voltage
void ADC_interrupt() {
    float V = readADC1_from_ISR();
    if ((V > THRESHOLD) & !state) {
        Timer.begin(updateDAC, DAC_UPDATE);
        state = true;
    } else if ((V < THRESHOLD) & state) {
        state = false;
    }
}

// Waveforms
float RAC_sinusoid(int trap_time, float amplitude, int offset) {
	float out;
	if (time < trap_time) {
		out = amplitude * sin(2*PI * time / PERIOD) + offset;
	} else {
		out = 0;
	}
	return out;
}

float cosPower(int start_time, int length, float amplitude, int power) {
	float out;
	if ((time >= start_time) && (time - start_time < length)) {
		// out = - amplitude * pow(cos(PI * (time - start_time) / length), power) + amplitude;
		float x = cos(PI * (time - start_time) / length);
		out = - amplitude * x * x * x * x * x * x * x * x + amplitude;
	} else {
		out = 0;
	}
	return out;
}

void setup() {
    configureADC(1, 1, 1, BIPOLAR_10V, ADC_interrupt);
}

void loop() {
}
