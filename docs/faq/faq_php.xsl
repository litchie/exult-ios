<?xml version="1.0"?> 
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="test.dtd">

<xsl:output method="xml"
	indent="no"/>

<!-- Keys -->
<xsl:key name="faq_ref" match="faq" use="@name"/>
<xsl:key name="section_ref" match="section" use="@title"/>

<xsl:strip-space elements="*"/>


<!-- Templates -->
<xsl:template match="faqs">
<test>
	<p>last changed: <xsl:value-of select="@changed"/></p>
	<hr/>
	<p>
		A text only version can be found <a href="http://exult.sourceforge.net/faq.txt">here</a> 
	</p>
	<br/>
	
	<!-- BEGIN TOC -->
	<xsl:for-each select="section">
		<p>
		<a href="#{generate-id(key('section_ref',@title))}">
			<xsl:number level="multiple"
						count="section"
						format="1. "
						value="position() -1"/>
				<xsl:value-of select="@title"/>
		</a>
		<br/>
		<xsl:for-each select="faq">
			<a href="#{generate-id(key('faq_ref',@name))}">
				<xsl:number level="multiple"
							count="section|faq"
							format="1."
							value="count(ancestor::section/preceding-sibling::section)"/>									
				<xsl:number format="1. "/>
				<xsl:apply-templates select="question"/>
			</a>
			<br/>
		</xsl:for-each>
		</p>
	</xsl:for-each>
	<!-- END TOC -->
	
	<!-- BEGIN CONTENT -->
	<xsl:apply-templates select="section"/>
	<!-- END CONTENT -->
</test>
</xsl:template>

<!-- FAQ Group Template -->
<xsl:template match="section">
	<hr width="100%"/>
	<table width="100%">
		<tr><th align="left">
			<a name="{generate-id()}">
				<xsl:number format="1. "
				value="position() -1"/>
				<xsl:value-of select="@title"/>
			</a>
		</th></tr>
		<xsl:apply-templates select="faq"/>
	</table>
</xsl:template>


<!-- FAQ Entry Template -->
<xsl:template match="faq">
	<xsl:variable name = "num_idx">
		<xsl:number level="single"
					count="section"					
					format="1."
					value="count(ancestor::section/preceding-sibling::section)"/>									
		<xsl:number format="1. "/>		
	</xsl:variable> 
	<tr><td><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td></tr>
	<tr><td><strong>
		<a name="{generate-id()}">
			<xsl:value-of select="$num_idx"/>
			<xsl:apply-templates select="question"/>
		</a>
	</strong></td></tr>
	<tr><td><xsl:apply-templates select="answer"/></td></tr>
</xsl:template>


<xsl:template match="question">
<!--
	<xsl:variable name = "data">
		<xsl:apply-templates/>
	</xsl:variable> 
	<xsl:value-of select="normalize-space($data)"/>
 -->
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="answer">
<!--
	<xsl:variable name = "data">
		<xsl:apply-templates/>
	</xsl:variable> 
	<xsl:value-of select="normalize-space($data)"/>
 -->
	<xsl:apply-templates/>
</xsl:template>


<!-- Internal Link Templates -->
<xsl:template match="ref">
	<a href="#{generate-id(key('faq_ref',@target))}">
	<xsl:choose>
		<xsl:when test="count(child::node())>0">
				<xsl:value-of select="."/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="count(key('faq_ref',@target)/parent::section/preceding-sibling::section)"/>
			<xsl:text>.</xsl:text>
			<xsl:value-of select="count(key('faq_ref',@target)/preceding-sibling::faq)+1"/>
			<xsl:text>.</xsl:text>					
		</xsl:otherwise>
	</xsl:choose>
	</a>
</xsl:template>


<xsl:template match="ref1">		
	<a href="#{generate-id(key('faq_ref',@target))}">		
		<xsl:value-of select="count(key('faq_ref',@target)/parent::section/preceding-sibling::section)"/>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="count(key('faq_ref',@target)/preceding-sibling::faq)+1"/>
		<xsl:text>. </xsl:text>
		<xsl:apply-templates select="key('faq_ref',@target)/child::question"/>
	</a>
</xsl:template>


<xsl:template match="ref2">		
	<a href="#{generate-id(key('section_ref',@target))}">		
		<xsl:value-of select="count(key('section_ref',@target)/preceding-sibling::section)"/>
		<xsl:text>. </xsl:text>
  		<xsl:apply-templates select="key('section_ref',@target)/@title"/>  		
	</a>
</xsl:template>


<!-- External Link Template -->
<xsl:template match="extref">
	<a href="{@target}">
	<xsl:choose>
		<xsl:when test="count(child::node())>0">
				<xsl:value-of select="."/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="@target"/>
		</xsl:otherwise>
	</xsl:choose>
	</a>
</xsl:template>


<!-- Misc Templates -->
<xsl:template match="Exult">
	<em>Exult</em>
</xsl:template>

<xsl:template match="Studio">
	<em>Exult Studio</em>
</xsl:template>

<xsl:template match="cite">
		<p>
		<xsl:value-of select="@name"/>:<br/>
		<cite><xsl:value-of select="."/></cite>
		</p>
</xsl:template>


<xsl:template match="para">
	<p><xsl:apply-templates/></p>
</xsl:template>


<xsl:template match="key">
	'<font color="maroon"><xsl:value-of select="."/></font>'
</xsl:template>


<xsl:template match="kbd">
	<font color="maroon"><kbd><xsl:value-of select="."/></kbd></font>
</xsl:template>


<!-- ...................ol|dl|ul + em............... -->
<xsl:template match="ul|ol|li|strong|q|br">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="em">
 <b><i><font size="+1">
<xsl:apply-templates/>
</font></i></b> 
</xsl:template>

<!-- Key Command Templates -->
<xsl:template match="keytable">
	<table border="0" cellpadding="0" cellspacing="2" width="80%">
		<tr>
			<th colspan="3" align="left">
				<xsl:value-of select="@title"/>
			</th>
		</tr>
		<xsl:apply-templates select="keydesc"/>
	</table>
</xsl:template>


<xsl:template match="keydesc">
	<tr>
		<td nowrap="nowrap" valign="top">
			<font color="maroon"><xsl:value-of select="@name"/></font>
		</td>
		<td width="10"><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
		<td><xsl:value-of select="."/></td>
	</tr>
</xsl:template>


<!-- Config Table Templates -->
<xsl:template match="configdesc">
	<table border="0" cellpadding="0" cellspacing="0">
		<xsl:apply-templates select="line"/>
	</table>
</xsl:template>


<xsl:template match="line">
	<tr>
	<xsl:choose>
		<xsl:when test="count(child::comment)>0">
			<td width="150">
				<xsl:value-of select="text()"/>
			</td>
			<xsl:apply-templates select="comment"/>
		</xsl:when>
		<xsl:otherwise>
			<td colspan="2"><xsl:value-of select="."/></td>
		</xsl:otherwise>
	</xsl:choose>
	</tr>
</xsl:template>


<xsl:template match="comment">
	<td rowspan="2">
		<xsl:apply-templates/>
	</td>
</xsl:template>



</xsl:stylesheet>