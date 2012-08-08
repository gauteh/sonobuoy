<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="yes" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="traco-electronic">
<description>&lt;b&gt;TRACO &lt;/b&gt;(R)&lt;b&gt; POWER&lt;/b&gt; DC DC Konverter&lt;p&gt;
www.tracopower.com&lt;p&gt;
Source:&lt;br&gt;
&lt;i&gt;traco.lbr&lt;/i&gt; from Bodo Fuhrmann  from Martin-Luther-Universität Halle, &amp;lt;bodo@biochemtech.uni-halle.de&amp;gt;&lt;br&gt;
&lt;i&gt;traco1.lbr&lt;/i&gt; from Hari Babu Kotte  from Mittuniversitetet,  &amp;lt;kotte_hari@rediffmail.com&amp;gt;&lt;br&gt;
Author: &lt;author&gt;librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="TRACOPOWER2W">
<description>&lt;b&gt;DC/DC-Konverter&lt;/b&gt; 2 Watt&lt;p&gt;
Source: http://www.tracopower.com/datasheet_g/tel2-d.pdf</description>
<wire x1="11.8" y1="-7" x2="11.8" y2="6.6" width="0.127" layer="21"/>
<wire x1="11.8" y1="-7" x2="-12" y2="-7" width="0.127" layer="21"/>
<wire x1="-12" y1="-7" x2="-12" y2="6.6" width="0.127" layer="21"/>
<wire x1="-12" y1="6.6" x2="11.8" y2="6.6" width="0.127" layer="21"/>
<circle x="-9.525" y="-3.81" radius="0.635" width="0" layer="21"/>
<pad name="16" x="-9.2" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="15" x="-6.6" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="14" x="-4.1" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="13" x="-1.6" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="12" x="1" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="11" x="3.6" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="10" x="6.1" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="9" x="8.6" y="4.8" drill="0.8" diameter="1.4"/>
<pad name="1" x="-9.2" y="-5.3" drill="0.8" diameter="1.4" shape="octagon"/>
<pad name="2" x="-6.6" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="3" x="-4.1" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="4" x="-1.6" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="5" x="1" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="6" x="3.5" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="7" x="6.1" y="-5.3" drill="0.8" diameter="1.4"/>
<pad name="8" x="8.6" y="-5.2" drill="0.8" diameter="1.4"/>
<text x="-8.6" y="-0.7" size="1.27" layer="21">DC DC CONVERTER</text>
</package>
</packages>
<symbols>
<symbol name="TRACOP1">
<wire x1="-10.16" y1="7.62" x2="-10.16" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-7.62" x2="10.16" y2="-7.62" width="0.254" layer="94"/>
<wire x1="10.16" y1="-7.62" x2="10.16" y2="7.62" width="0.254" layer="94"/>
<wire x1="10.16" y1="7.62" x2="-10.16" y2="7.62" width="0.254" layer="94"/>
<text x="-10.16" y="8.89" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-10.16" size="1.778" layer="96">&gt;VALUE</text>
<pin name="+VIN" x="-12.7" y="5.08" length="short" direction="in"/>
<pin name="-VIN" x="-12.7" y="-5.08" length="short" direction="in"/>
<pin name="+VOUT" x="12.7" y="5.08" length="short" direction="pas" rot="R180"/>
<pin name="COM" x="12.7" y="0" length="short" direction="pas" rot="R180"/>
<pin name="-VOUT" x="12.7" y="-5.08" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="TEL_2-" prefix="PWS">
<description>&lt;b&gt;DC/DC-Konverter&lt;/b&gt; 2 Watt, 1 x Input, 1 x +- Output&lt;p&gt;
Source: http://www.tracopower.com/datasheet_g/tel2-d.pdf</description>
<gates>
<gate name="G$1" symbol="TRACOP1" x="-5.08" y="2.54"/>
</gates>
<devices>
<device name="" package="TRACOPOWER2W">
<connects>
<connect gate="G$1" pin="+VIN" pad="16"/>
<connect gate="G$1" pin="+VOUT" pad="9"/>
<connect gate="G$1" pin="-VIN" pad="1"/>
<connect gate="G$1" pin="-VOUT" pad="10"/>
<connect gate="G$1" pin="COM" pad="8"/>
</connects>
<technologies>
<technology name="0521">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-0521" constant="no"/>
<attribute name="OC_FARNELL" value="1007027" constant="no"/>
<attribute name="OC_NEWARK" value="51R4755" constant="no"/>
</technology>
<technology name="0522">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-0522" constant="no"/>
<attribute name="OC_FARNELL" value="1007028" constant="no"/>
<attribute name="OC_NEWARK" value="51R4756" constant="no"/>
</technology>
<technology name="0523">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-0523" constant="no"/>
<attribute name="OC_FARNELL" value="1007029" constant="no"/>
<attribute name="OC_NEWARK" value="51R4757" constant="no"/>
</technology>
<technology name="1221">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-1221" constant="no"/>
<attribute name="OC_FARNELL" value="1007032" constant="no"/>
<attribute name="OC_NEWARK" value="51R4760" constant="no"/>
</technology>
<technology name="1222">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-1222" constant="no"/>
<attribute name="OC_FARNELL" value="1007033" constant="no"/>
<attribute name="OC_NEWARK" value="51R4761" constant="no"/>
</technology>
<technology name="1223">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-1223" constant="no"/>
<attribute name="OC_FARNELL" value="1007034" constant="no"/>
<attribute name="OC_NEWARK" value="51R4762" constant="no"/>
</technology>
<technology name="2421">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-2421" constant="no"/>
<attribute name="OC_FARNELL" value="1007039" constant="no"/>
<attribute name="OC_NEWARK" value="51R4765" constant="no"/>
</technology>
<technology name="2422">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-2422" constant="no"/>
<attribute name="OC_FARNELL" value="1007041" constant="no"/>
<attribute name="OC_NEWARK" value="51R4766" constant="no"/>
</technology>
<technology name="2423">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-2423" constant="no"/>
<attribute name="OC_FARNELL" value="1007042" constant="no"/>
<attribute name="OC_NEWARK" value="51R4767" constant="no"/>
</technology>
<technology name="4821">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-4821" constant="no"/>
<attribute name="OC_FARNELL" value="1007045" constant="no"/>
<attribute name="OC_NEWARK" value="51R4770" constant="no"/>
</technology>
<technology name="4822">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-4822" constant="no"/>
<attribute name="OC_FARNELL" value="1007046" constant="no"/>
<attribute name="OC_NEWARK" value="51R4771" constant="no"/>
</technology>
<technology name="4823">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="TEL 2-4823" constant="no"/>
<attribute name="OC_FARNELL" value="1007047" constant="no"/>
<attribute name="OC_NEWARK" value="51R4772" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="PWS1" library="traco-electronic" deviceset="TEL_2-" device="" technology="1222"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="PWS1" gate="G$1" x="-50.8" y="40.64"/>
</instances>
<busses>
</busses>
<nets>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
