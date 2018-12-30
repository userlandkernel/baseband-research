<interface name="timer">
  <!-- uint64_t get_ticks (); -->
  <method rtype="uint64_t" name="get_ticks" />

  <!-- void set_ticks (uint64_t ticks); -->
  <method rtype="void" name="set_ticks">
   <arg atype="uint64_t" name="ticks" />
  </method>

  <!-- uint64_t get_tick_frequency (); -->
  <method rtype="uint64_t" name="get_tick_frequency" />

  <!-- uint64_t set_tick_frequency (uint64_t frequency); -->
  <method rtype="uint64_t" name="set_tick_frequency">
   <arg atype="uint64_t" name="frequency" />
  </method>

  <!-- int timeout (uint64_t); -->
  <method rtype="int" name="timeout">
   <arg atype="uint64_t" name="ticks" />
  </method>
</interface>

