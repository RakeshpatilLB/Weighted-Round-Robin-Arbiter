#include "NGPRC.h"
 void NGPRC::prc_preCalStateTransition(){if (reset.read()) {
        state = RESET;
    } else {
        switch (state) {
            case RESET: 
                state = NEXT_GRANT; 
                break;
            case NEXT_GRANT:
                // No state change needed
                break;
            default: 
                state = RESET; 
                break;
        }
    }}
  void NGPRC::prc_preCalOutputLogic(){if (reset.read()) {
        nextGrant.write(sc_uint<CHANNELS>(0));  // Proper initialization with 0
        priorityMask.write(sc_uint<CHANNELS>(0));    // Proper initialization with 0
    } else {
        switch (state) {
            case RESET: {
                nextGrant.write(sc_uint<CHANNELS>(0));
                priorityMask.write(sc_uint<CHANNELS>(0));
                break;
            }
            case NEXT_GRANT: {
                // Proper bit concatenation in SystemC for left rotation
                sc_uint<CHANNELS> temp = grant.read();
                sc_uint<CHANNELS> rotated;
                
                // Perform left rotation
                rotated.range(CHANNELS-1, 1) = temp.range(CHANNELS-2, 0);
                rotated[0] = temp[CHANNELS-1];
                
                // Write to concat_grant signal
                concat_grant.write(rotated);
                
                // Calculate priority mask
              unsigned int rotated_uint = rotated.to_uint();
			  sc_uint<CHANNELS> mask = sc_uint<CHANNELS>((~rotated_uint) + 1);

                if (mask == 0) {
                    mask = ~sc_uint<CHANNELS>(0);  // All 1s
                }
                priorityMask.write(mask);
                
                // Calculate next grant
                sc_uint<CHANNELS> next = request.read() & mask;
                nextGrant.write(next);
                
                // Handle the case when no grants match the priority mask
                if ((next == 0) && (request.read() != 0)) {
                    nextGrant.write(request.read());
                }
                break;
            }
            default: {
                // No change needed
                break;
            }
        }
    }}
