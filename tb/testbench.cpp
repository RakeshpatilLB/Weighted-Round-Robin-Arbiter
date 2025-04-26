#include <systemc.h>
#include "design.cpp"  
//const int CHANNELS = 4; already defined in NGPRC.h
const int WIDTH = 4;    // Width of the weight bus (can be modified)
const int DEC_SEL_WIDTH = 2;  // Width of the decoder output (log2(CHANNELS))
SC_MODULE(Testbench) {
    // Signals
    sc_clock clk;
    sc_signal<bool> reset;
    sc_signal<sc_uint<CHANNELS>> request;
    sc_signal<sc_uint<CHANNELS * WIDTH>> weight;
    sc_signal<sc_uint<CHANNELS>> grant;
    RRBTOP<CHANNELS, WIDTH, DEC_SEL_WIDTH>* top;
    
    // Monitor process for printing signal values
    void monitor() {
        while (true) {
            wait(clk.posedge_event());  // Wait for positive edge of clock
            
            // Print the current values of all signals
            std::cout << "Time: " << sc_time_stamp() << std::endl;
            std::cout << "  Reset: " << reset.read() << std::endl;
            std::cout << "  Request: 0x" << std::hex << request.read() << std::dec << std::endl;
            std::cout << "  Weight: 0x" << std::hex << weight.read() << std::dec << std::endl;
            std::cout << "  Grant: 0x" << std::hex << grant.read() << std::dec << std::endl;
            std::cout << "----------------------------------------" << std::endl;
        }
    }
    
    void stimulus() {
        // Reset the system
        reset.write(true);
        wait(2); // Wait for 2 clock cycles
        reset.write(false);

        // Test 1: Apply a request to channel 0 and 3 with some weights
        request.write(0xf);  // Binary 1001 - Channel 0 and 3 are requesting
        weight.write(0x1111); // Weight pattern for each channel (32-bit packed)
        wait(200);  // Run for 10 clock cycles
        // Test 2: Apply a request to channel 1
        request.write(0xf);  // Channel 1 is requesting
        weight.write(0x1312);  // Same weight pattern
        wait(200);
        //wait some more
      	wait(100);
         request.write(0x0);
	     weight.write(0x0000);
              wait(100);

        // End simulation
        sc_stop();
    }
    
    SC_CTOR(Testbench)
        : clk("clk", 10, SC_NS)  // 100 MHz clock
    {
        // Instantiate the design under test (DUT)
        top = new RRBTOP<CHANNELS, WIDTH, DEC_SEL_WIDTH>("RRBTOP");
        top->clk(clk);
        top->reset(reset);
        top->request(request);
        top->weight(weight);
        top->grant(grant);
        
        SC_THREAD(stimulus);
        sensitive << clk;  
        // Correct method for positive edge sensitivity
        //using clk.pos() throws error
        
        // Add monitor thread
        SC_THREAD(monitor);
    }
    
    ~Testbench() {
        delete top;
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");
    // VCD tracing
    sc_trace_file* tf = sc_create_vcd_trace_file("dump");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, tb.clk, "clk");
    sc_trace(tf, tb.reset, "reset");
    sc_trace(tf, tb.request, "request");
    sc_trace(tf, tb.weight, "weight");
    sc_trace(tf, tb.grant, "grant");
    sc_start();  // Start the simulation
    sc_close_vcd_trace_file(tf); 
  // Close the VCD trace file after the simulation ends
    return 0;
}
