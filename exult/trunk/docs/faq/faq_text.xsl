<?xml version="1.0"?> 
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">


<xsl:strip-space elements="*"/>
<xsl:output
	method="text"
	indent="no"
	encoding="iso-8859-1"/>

<!-- Keys -->
<xsl:key name="faq_ref" match="faq" use="@name"/>
<xsl:key name="section_ref" match="section" use="@title"/>


<!-- Templates -->
<xsl:template match="faqs">
	<xsl:value-of select="@title"/>
	<xsl:text>F.A.Q. (frequently asked questions)&#xA;</xsl:text>
	<xsl:text>last changed: </xsl:text>
	<xsl:value-of select="@changed"/>
	<xsl:text>&#xA;&#xA;</xsl:text>
	<xsl:text>A html version can be found at http://exult.sourceforge.net/faq.shtml&#xA;</xsl:text>
	<xsl:text>&#xA;&#xA;</xsl:text>

	<!-- BEGIN TOC -->
	<xsl:for-each select="section">
		<xsl:number level="multiple" 
					count="section" 
					format="1. "
					value="position() -1"/>
		<xsl:value-of select="@title"/><xsl:text>&#xA;</xsl:text>
	<xsl:for-each select="faq">
		<xsl:number level="multiple" 
					count="section|faq" 
					format="1."
					value="count(ancestor::section/preceding-sibling::section)"/>
		<xsl:number format="1. "/>
		<xsl:apply-templates select="question"/><xsl:text>&#xA;</xsl:text>
		</xsl:for-each>
		<xsl:text>&#xA;</xsl:text>
	</xsl:for-each>
	<!-- END TOC -->

	<!-- BEGIN CONTENT -->
	<xsl:apply-templates select="section"/>
	<!-- END CONTENT -->
</xsl:template>

<!-- FAQ section Template -->
<xsl:template match="section">
	<xsl:text>&#xA;</xsl:text>
	<xsl:text>--------------------------------------------------------------------------------&#xA;</xsl:text>
	<xsl:text>&#xA;</xsl:text>
	<xsl:number format="1. "/><xsl:value-of select="@title"/>
	<xsl:text>&#xA;</xsl:text>
	<xsl:apply-templates select="faq"/>
</xsl:template>


<!-- FAQ Entry Template -->
<xsl:template match="faq">
	<xsl:variable name = "num_idx">
		<xsl:number level="multiple"
					count="section|faq"
					format="1.1 "/>
	</xsl:variable> 
	<xsl:value-of select="$num_idx"/><xsl:apply-templates select="question"/>
	<xsl:text>&#xA;</xsl:text>
	<xsl:apply-templates select="answer"/>
	<xsl:text>&#xA;&#xA;</xsl:text>
</xsl:template>


<xsl:template match="question">
		<xsl:apply-templates/>
<!-- normalize-space -->
</xsl:template>


<xsl:template match="answer">
		<xsl:apply-templates/>
</xsl:template>


<!-- Internal Link Templates -->
<xsl:template match="ref">
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
</xsl:template>

<xsl:template match="ref1">		
		<xsl:text>&#xA;</xsl:text>
		<xsl:value-of select="count(key('faq_ref',@target)/parent::section/preceding-sibling::section)"/>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="count(key('faq_ref',@target)/preceding-sibling::faq)+1"/>
		<xsl:text>. </xsl:text>
		<xsl:apply-templates select="key('faq_ref',@target)/child::question"/>
</xsl:template>


<xsl:template match="ref2">				
		<xsl:text>&#xA;</xsl:text>
		<xsl:value-of select="count(key('section_ref',@target)/preceding-sibling::section)"/>
		<xsl:text>. </xsl:text>
  		<xsl:apply-templates select="key('section_ref',@target)/@title"/>  		
</xsl:template>

<!-- External Link Template -->
<xsl:template match="extref">
	<xsl:choose>
		<xsl:when test="count(child::node())>0">
				<xsl:value-of select="."/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="@target"/>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>


<!-- Misc Templates -->
<xsl:template match="Exult">
	<xsl:text>Exult</xsl:text>
</xsl:template>


<xsl:template match="cite">
		<p>
		<xsl:value-of select="@name"/>:<br/>
		<cite><xsl:value-of select="."/></cite>
		</p>
</xsl:template>


<xsl:template match="key">
	'<xsl:value-of select="."/>'
</xsl:template>


<xsl:template match="para">
	<xsl:variable name = "data">
		<xsl:apply-templates/>
	</xsl:variable> 
	<xsl:value-of select="$data"/>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="br">
	<xsl:text>&#xA;</xsl:text>
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
	<xsl:choose>
		<xsl:when test="count(child::comment)>0">
			<xsl:value-of select="text()"/>
			<xsl:apply-templates select="comment"/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="."/>
		</xsl:otherwise>
	</xsl:choose>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>


<xsl:template match="comment">
	<td rowspan="2">
		<xsl:value-of select="."/>
	</td>
</xsl:template>


<!-- 
<xsl:template match="text()">
	<xsl:value-of select="."/>
</xsl:template>
-->

<!-- Clone template. Allows one to use any XHTML in the source file -->
<!-- 
<xsl:template match="@*|node()">
	<xsl:copy>
		<xsl:apply-templates select="@*|node()"/>
	</xsl:copy>
</xsl:template>
-->

</xsl:stylesheet>
