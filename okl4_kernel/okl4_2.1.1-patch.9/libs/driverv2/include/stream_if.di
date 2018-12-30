<interface name="stream">
  <datafield ftype="struct stream_pkt *" name="start" />
  <datafield ftype="struct stream_pkt *" name="end" />

  <datafield ftype="struct stream_pkt *" name="completed_start" />
  <datafield ftype="struct stream_pkt *" name="completed_end" />

  <method rtype="int" name="sync" />
</interface>
