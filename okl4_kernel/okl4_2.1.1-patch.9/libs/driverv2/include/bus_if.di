<interface name="bus">
  <!-- uint32_t execute(struct cmd_packet *argv); -->


  <method rtype="int" name="write8">
    <arg atype="int" name="offset" />
    <arg atype="uint8_t" name="val" />
  </method>

  <method rtype="int" name="write16">
    <arg atype="int" name="offset" />
    <arg atype="uint16_t" name="val" />
  </method>

  <method rtype="int" name="write32">
    <arg atype="int" name="offset" />
    <arg atype="uint32_t" name="val" />
  </method>

  <method rtype="int" name="read8">
    <arg atype="int" name="offset" />
    <arg atype="uint8_t*" name="val" />
  </method>

  <method rtype="int" name="read16">
    <arg atype="int" name="offset" />
    <arg atype="uint16_t*" name="val" />
  </method>

  <method rtype="int" name="read32">
    <arg atype="int" name="offset" />
    <arg atype="uint32_t*" name="val" />
  </method>

  <!-- data associated with interface -->
</interface>

