/***
 * A work in progress.
 *
 * Combines the basic_ddc, ddc_invalid, and seal examples.
 *
 * The original intent was to show how to foil attempts 
 * at writing an invalid capability to the ddc
 * by first sealing it, then trying to clear the tag
 * before writing it.
 *
 * Turns out it crashes when we try to write the sealed
 * capability to the ddc anyway, but I'm not sure yet if that's
 * because of the sealing, or the fragility of the basic_ddc example.
 * Also not sure if it does only work on hybrid
 ***/
 
#include "../include/common.h"
#include "include/utils.h"
#include <assert.h>
#include <cheriintrin.h>
#include <machine/sysarch.h> 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>

#define CHERI_TOCAP __cheri_tocap
 
 #if !defined(__CHERI_CAPABILITY_WIDTH__) || defined(__CHERI_PURE_CAPABILITY__)
 #error "This example only works on CHERI hybrid mode"
 #endif
 
 int main()
 {
     // start with a void pointer and malloc enough size for a uint16_t.
     void *ptr_16 = malloc(sizeof(uint16_t));
     // TODO do I need to do it this way? I think I could've just cast it straight
     void * __capability alpha_ptr = (CHERI_TOCAP void * __capability)ptr_16;
     
     // now make sure the ddc's address isn't the same as the alpha pointer
     assert(cheri_address_get(cheri_ddc_get()) != cheri_address_get((void *__capability) alpha_ptr));
     // show the details
     pp_cap(alpha_ptr);


     // now create a second pointer and malloc enough space
     void *ptr_32 = malloc(sizeof(uint32_t));
     // TODO I think I can just straight cast this
     void * __capability bravo_ptr = (CHERI_TOCAP void * __capability)ptr_32;
     
     // now we combine the invalid example, first ensure we have
     // a valid tag on the bravo pointer
     assert(cheri_tag_get(bravo_ptr));
     // now seal the bravo pointer
     void *sealptr;
     // TODO I can just straight cast this
     void * __capability sealcap = (CHERI_TOCAP void * __capability)sealptr;
     size_t sealcap_size = sizeof(sealcap);
     
     if (sysctlbyname("security.cheri.sealcap", &sealcap, &sealcap_size, NULL, 0) < 0)
     {
        error("Fatal error. Cannot get 'security.cheri.sealcap'.");
        exit(1);
     }
     
     void *sealbravo;
     size_t sealbravo_size = sizeof(bravo_ptr);
     // TODO I can just straight cast this
     void * __capability bravosealed = (CHERI_TOCAP void * __capability)sealbravo;
     bravosealed = cheri_seal(bravo_ptr, sealcap);
     
     // TODO bravo_ptr isn't sealed, right?
     printf("About to try to write (unsealed) bravo_ptr to ddc\n");
     write_ddc((void *__capability) bravo_ptr);
    
     
     printf("About to try to write bravosealed to ddc\n");
     write_ddc((void *__capability) bravosealed);
     
     // It shouldn't get any further than this, so the rest is moot
     
     printf("About to try to clear the tag on bravosealed\n");
     bravosealed = cheri_tag_clear(bravosealed);
     
     printf("About to try to write the cleared (invalid) bravosealed to ddc\n");
     write_ddc((void *__capability) bravosealed);
     
     // now flip the ddc to point to the alpha pointer, changing compartments
     void *sealalpha;
     size_t sealalpha_size = sizeof(alpha_ptr);
     
     // now make sure the ddc's address isn't the same as the alpha pointer
     assert(cheri_address_get(cheri_ddc_get()) != cheri_address_get((void *__capability) alpha_ptr));
     // and make sure the ddc's address IS the same as the bravo pointer
     assert(cheri_address_get(cheri_ddc_get()) == cheri_address_get((void *__capability) bravo_ptr));
     
     // seal the alpha poiter
     void * __capability alphasealed = (CHERI_TOCAP void * __capability)sealalpha;
     alphasealed = cheri_seal(alpha_ptr, sealcap);
     printf("About to try to clear the tag on alphasealed\n");
     alphasealed = cheri_tag_clear(alphasealed);
     
     printf("About to try to write (unsealed) alpha pointer to ddc\n");
     write_ddc((void *__capability) alpha_ptr);
     
     // now make sure the ddc's address ISN'T the same as the bravo pointer (because
     // we switched compartments)
     assert(cheri_address_get(cheri_ddc_get()) != cheri_address_get((void *__capability) bravo_ptr));
     // and make sure the ddc's address IS the same as the alpha pointer
     // because we switched back to it
     assert(cheri_address_get(cheri_ddc_get()) == cheri_address_get((void *__capability) alpha_ptr));
     // craaaaaaash
     printf("About to try to write alphasealed to ddc\n");
     write_ddc(alphasealed);
     
     return 0;
     
     
 }