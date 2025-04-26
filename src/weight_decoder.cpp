#include <systemc.h>
// One-Hot to Decimal Converter
template <int CHANNELS, int DEC_SEL_WIDTH>
SC_MODULE(OneHotToDecimal) {
    sc_in<sc_uint<CHANNELS>> one_hot_in;
    sc_out<sc_uint<DEC_SEL_WIDTH>> decimal_out;
  
  sc_signal<sc_uint<DEC_SEL_WIDTH>> result; 

    void convert() {
        sc_uint<DEC_SEL_WIDTH> result = 0;
        for (int i = 0; i < CHANNELS; ++i) {
            if (one_hot_in.read()[i]) {
                result = i;
                break;
            }
        }
        decimal_out.write(result);
    }

    SC_CTOR(OneHotToDecimal) {
        SC_METHOD(convert);
        sensitive << one_hot_in;
    }
};

// Top-Level Mux Module (Packed I/O)
template <int CHANNELS, int WIDTH, int DEC_SEL_WIDTH>
SC_MODULE(Mux) {
    sc_in<sc_uint<CHANNELS>> one_hot_sel;
    sc_in<sc_uint<CHANNELS * WIDTH>> data_in;
    sc_out<sc_uint<WIDTH>> data_out;

    sc_signal<sc_uint<DEC_SEL_WIDTH>> decoded_sel;

    // Decoder instance
    OneHotToDecimal<CHANNELS, DEC_SEL_WIDTH>* decoder;

    void mux_logic() {
        sc_uint<WIDTH> inputArray[CHANNELS];
        sc_uint<CHANNELS * WIDTH> full_input = data_in.read();

        // Unpack input vector to array
        for (int i = 0; i < CHANNELS; ++i) {
            inputArray[i] = full_input.range((i + 1) * WIDTH - 1, i * WIDTH);
        }

        // Apply selection
        sc_uint<DEC_SEL_WIDTH> sel = decoded_sel.read();
        if (sel < CHANNELS)
            data_out.write(inputArray[sel]);
        else
            data_out.write(0);  // Default fallback
    }

    SC_CTOR(Mux) {
        decoder = new OneHotToDecimal<CHANNELS, DEC_SEL_WIDTH>("decoder");
        decoder->one_hot_in(one_hot_sel);
        decoder->decimal_out(decoded_sel);

        SC_METHOD(mux_logic);
        sensitive << data_in << decoded_sel;
    }

    ~Mux() {
        delete decoder;
    }
};
