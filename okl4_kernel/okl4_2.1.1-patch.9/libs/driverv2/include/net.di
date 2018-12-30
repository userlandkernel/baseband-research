<interface name="net">
   <!-- Setup the structure, but do not touch or enable the device -->
   <method rtype="int" name="read_mac">
    <arg atype="uint16_t*" name="mac1" />
    <arg atype="uint16_t*" name="mac2" />
    <arg atype="uint16_t*" name="mac3" />
   </method>

   <method rtype="int" name="set_promiscious">
    <arg atype="uint32_t" name="op" />
   </method>

   <method rtype="int" name="add_packet">
    <arg atype="struct client_cmd_packet *" name="packet" />
   </method>

</interface>

