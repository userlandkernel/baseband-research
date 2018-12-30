<interface name="rtc">

  <!--                     -->
  <!-- Wallclock           -->
  <!--                     -->

  <!-- uint64_t get_time (); -->
  <method rtype="uint64_t" name="get_time" />

  <!-- uint64_t set_time (uint64_t ticks); -->
  <method rtype="void" name="set_time">
   <arg atype="uint64_t" name="time" />
  </method>

  <!--                     -->
  <!-- Alarm               -->
  <!--                     -->

  <!-- uint64_t get_alarm (); -->
  <method rtype="uint64_t" name="get_alarm" />

  <!-- int set_alarm (uint64_t ticks);   -->
  <!-- returns 1 if alarm has activated. -->
  <method rtype="int" name="set_alarm">
   <arg atype="uint64_t" name="time" />
  </method>

  <!-- void cancel_alarm (); -->
  <method rtype="void" name="cancel_alarm" />

</interface>

