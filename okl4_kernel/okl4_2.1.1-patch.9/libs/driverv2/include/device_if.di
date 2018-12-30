<interface name="device">
   <!-- Setup the structure, but do not touch or enable the device -->
   <method rtype="int" name="setup">
    <arg atype="struct resource *" name="resources" />
   </method>

   <!-- Enable the device. Return ENABLE_COMPLETE or CONTINUE -->
   <method rtype="int" name="enable" />

   <!-- Enable the device. Return DISABLE_COMPLETE or CONTINUE -->
   <method rtype="int" name="disable" />

   <!-- Enable the device. Return DISABLE_COMPLETE, ENABLE_COMPLETE, maybe interface specific or CONTINUE -->
   <method rtype="int" name="poll" />

   <!-- Enable the device. Return DISABLE_COMPLETE, ENABLE_COMPLETE, maybe interface specific or CONTINUE -->
   <method rtype="int" name="interrupt">
    <arg atype="int" name="interrupt" />
   </method>

   <method rtype="int" name="num_interfaces" />

   <method rtype="struct generic_interface *" name="get_interface"> 
     <arg atype="int" name="interface_num" />
   </method>

</interface>
