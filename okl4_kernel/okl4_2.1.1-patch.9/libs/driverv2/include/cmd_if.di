<interface name="cmd">
  <!-- uint32_t execute(struct cmd_packet *argv); -->
  <method rtype="int" name="execute">
    <arg atype="struct cmd_packet *" name="packet" />
  </method>

  <!-- data associated with interface -->
  <datafield ftype="struct okl4_mutex" name="queue_lock" />
  <datafield ftype="struct cmd_packet *" name="free_list" />
  <datafield ftype="struct cmd_packet *" name="pend_list" />
  <datafield ftype="struct cmd_packet *" name="done_list" />
</interface>

