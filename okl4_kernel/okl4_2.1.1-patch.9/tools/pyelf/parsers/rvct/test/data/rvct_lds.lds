#line 1 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"




























 
#line 1 "alt/qct/pistachio/include/l4.h"
 
 


 




#line 1 "alt/qct/pistachio/include/macros.h"
 
 


 




#line 1 "build/pistachio/include/compat/c.h"
 
 


 



#line 1 "build/pistachio/include/compat/toolchain/rvct_gnu/c.h"
 
 


 





 
#line 23 "build/pistachio/include/compat/toolchain/rvct_gnu/c.h"



 
#line 33 "build/pistachio/include/compat/toolchain/rvct_gnu/c.h"



 






 


#line 17 "build/pistachio/include/compat/c.h"
#line 11 "alt/qct/pistachio/include/macros.h"












 

 
#line 33 "alt/qct/pistachio/include/macros.h"












 














 








 




 




 
#line 99 "alt/qct/pistachio/include/macros.h"

 






 



 







#line 164 "alt/qct/pistachio/include/macros.h"

#line 11 "alt/qct/pistachio/include/l4.h"
#line 1 "alt/qct/pistachio/include/types.h"
 
 


 




#line 142 "alt/qct/pistachio/include/types.h"

#line 12 "alt/qct/pistachio/include/l4.h"
#line 1 "alt/qct/pistachio/include/config.h"
 
 
 


 



 












 








#line 45 "alt/qct/pistachio/include/config.h"

#line 52 "alt/qct/pistachio/include/config.h"



 









#line 1 "alt/qct/pistachio/arch/arm/include/arch/config.h"
 
 


 




#line 1 "alt/qct/pistachio/include/l4.h"
 
 


 

#line 11 "alt/qct/pistachio/arch/arm/include/arch/config.h"
#line 1 "alt/qct/pistachio/arch/arm/include/arch/page.h"
 
 


 











 













 


#line 45 "alt/qct/pistachio/arch/arm/include/arch/page.h"











#line 68 "alt/qct/pistachio/arch/arm/include/arch/page.h"



















 






#line 12 "alt/qct/pistachio/arch/arm/include/arch/config.h"










#line 29 "alt/qct/pistachio/arch/arm/include/arch/config.h"




 
#line 40 "alt/qct/pistachio/arch/arm/include/arch/config.h"









 






 


 




 





 









 




 







 
#line 1 "alt/qct/pistachio/arch/arm/v5/include/arch/ver/config.h"
 
 


 






 



 




 


 










 
 
#line 43 "alt/qct/pistachio/arch/arm/v5/include/arch/ver/config.h"



#line 95 "alt/qct/pistachio/arch/arm/include/arch/config.h"



 





 



 



#line 125 "alt/qct/pistachio/arch/arm/include/arch/config.h"

#line 146 "alt/qct/pistachio/arch/arm/include/arch/config.h"

#line 177 "alt/qct/pistachio/arch/arm/include/arch/config.h"

 




















 


 











 

 









 





 





 





 









 
 





 



 

 




 
#line 282 "alt/qct/pistachio/arch/arm/include/arch/config.h"






















 



 




 




 


#line 1 "alt/qct/pistachio/platform/versatile/include/plat/timer.h"
 
 


 






#line 322 "alt/qct/pistachio/arch/arm/include/arch/config.h"

#line 66 "alt/qct/pistachio/include/config.h"

#line 13 "alt/qct/pistachio/include/l4.h"

#line 31 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"
#line 1 "build/pistachio/object/pistachio/include/tcb_layout.h"
 




#line 53 "build/pistachio/object/pistachio/include/tcb_layout.h"

#line 32 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"
#line 1 "build/pistachio/object/pistachio/include/asmsyms.h"
 



#line 36 "build/pistachio/object/pistachio/include/asmsyms.h"

#line 33 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"
#line 1 "alt/qct/pistachio/platform/versatile/include/plat/offsets.h"
 
 


 




#line 34 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"

LR_1 0xF0000000 NOCOMPRESS
{
    ER_START_ROM 0xF0000000
    {
	* (._start_rom)
    }

    ER_ROOT +0
    {
	* (.init.head)
    }

    ER_RO +0
    {
	.ANY (+RO)
    }

    ER_UNUSED +0
    {
	* (.delete)
    }

    ER_END_ROM +0
    {
	* (._end_rom)
    }

    ER_INIT +0
    {
	* (._start_init, +FIRST)
	* (.init)
	* (.roinit)
	* (._end_init, +LAST)
    }

#line 77 "alt/qct/pistachio/arch/arm/src/rvct/linker.lds"
    ER_START_RAM +0

    {
	* (._start_ram)
    }

    ER_GOT +0 EMPTY 256
    {
    }

    ER_RW +0
    {
	.ANY (+RW)
    }

    ER_ZI +0
    {
	* (._start_bss, +FIRST)
	.ANY (+ZI)
	* (._end_bss, +LAST)
    }

    ER_KSPACE +0
    {
	* (.data.kspace)
    }

    .kip +0
    {
	* (.data.kip, +FIRST)
	* (.data.kip.pdesc)
	* (.data.kip.kdesc, +LAST)
    }

    .kip.features +0
    {
	* (.data.kip.versionparts, +FIRST)
	* (.data.kip.features)
	* (.data.kip.features.end, +LAST)
    }

    .kip.user +0
    {
	user.o (.user.*)
	user_thumb.o (.user_thumb.*)
    }

    .kip.mdesc +0
    {
	* (._start_mdesc, +FIRST)
	* (.data.kip.mdesc)
	* (._end_kip, +LAST)
    }

    ER_END_RAM +0
    {
	* (._end_ram, +FIRST)
	* (._end, +LAST)
    }
}
