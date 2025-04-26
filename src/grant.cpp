#include <systemc.h>
#include "weight_decoder.cpp"
template<int CHANNELS, int WEIGHT_WIDTH, int DEC_SEL_WIDTH>
SC_MODULE(GrantModule) {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<sc_uint<CHANNELS>> request;
    sc_in<sc_uint<CHANNELS>> nextGrant;
    sc_in<sc_uint<CHANNELS * WEIGHT_WIDTH>> weight; // Packed weights bus
    sc_out<sc_uint<CHANNELS>> grant;
    
    // Internal signals
    sc_signal<sc_uint<CHANNELS>> currentGrant;
    sc_signal<sc_uint<WEIGHT_WIDTH>> weightCounter;
    sc_signal<sc_uint<WEIGHT_WIDTH>> selected_weight; // Output from internal MUX
    enum State { IDLE, GRANT, COUNT };
    sc_signal<State> state;
    
    // Internal MUX instance
    Mux<WEIGHT_WIDTH, CHANNELS, DEC_SEL_WIDTH>* weight_mux;
    
    void grant_fsm() {
        if (reset.read()) {
            state.write(IDLE);
            currentGrant.write(0);
            weightCounter.write(0);
            grant.write(0);
        } else {
            switch (state.read()) {
                case IDLE:
                    if (request.read() != 0) {
                        sc_uint<CHANNELS> nextGrantVal = request.read() & nextGrant.read();
                        if (nextGrantVal == 0) {
                            nextGrantVal = request.read();  // wrap-around case
                        }
                        // Extract the index of the first '1' bit
                        for (int i = 0; i < CHANNELS; ++i) {
                            if (nextGrantVal[i]) {
                                currentGrant.write(1 << i);
                                break;
                            }
                        }
                        state.write(GRANT);
                    }
                    break;
                case GRANT:
                    grant.write(currentGrant.read());
                    weightCounter.write(selected_weight.read()); // Use weight from internal MUX
                    state.write(COUNT);
                    break;
                case COUNT:
                    if (weightCounter.read() == 0) {
                        grant.write(0);
                        state.write(IDLE);
                    } else {
                        weightCounter.write(weightCounter.read() - 1);
                    }
                    break;
            }
        }
    }
    
    SC_CTOR(GrantModule) {
        // Create internal MUX
        weight_mux = new Mux<WEIGHT_WIDTH, CHANNELS, DEC_SEL_WIDTH>("weight_mux");
        weight_mux->one_hot_sel(currentGrant); // Select based on current grant
        weight_mux->data_in(weight);           // Input is packed weights
        weight_mux->data_out(selected_weight); // Output is selected weight
        
        SC_METHOD(grant_fsm);
        sensitive << clk.pos() << reset.pos();
    }
    
    ~GrantModule() {
        delete weight_mux;
    }
};
