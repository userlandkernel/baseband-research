<interface name="fb">
   <!-- Setup the structure, but do not touch or enable the device -->

   <method rtype="uintptr_t" name="get_buffer">
   </method>

   <method rtype="void" name="set_buffer">
    <arg atype="uintptr_t" name="buff_phys" />
   </method>

   <method rtype="void" name="get_mode">
    <arg atype="uint32_t*" name="xres" />
    <arg atype="uint32_t*" name="yres" />
    <arg atype="uint32_t*" name="bpp" />
   </method>

   <method rtype="int" name="set_mode">
    <arg atype="uint32_t" name="xres" />
    <arg atype="uint32_t" name="yres" />
    <arg atype="uint32_t" name="bpp" />
   </method>


</interface>

