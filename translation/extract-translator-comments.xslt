<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >
<xsl:output method="text" encoding="utf-8"/>

<!-- The following strips spaces from all nodes -->
<xsl:strip-space elements="*" />

<xsl:template match="context">
	<xsl:apply-templates select="message"/>
</xsl:template>

<xsl:template match="message">
	<xsl:if test="normalize-space(translatorcomment) != ''">
		<xsl:text>[ </xsl:text><xsl:value-of select="normalize-space(source)"/><xsl:text> ] =====>>>>> </xsl:text>
		<xsl:value-of select="normalize-space(translatorcomment)"/>
		<xsl:text>&#10;&#10;</xsl:text>
	</xsl:if>
</xsl:template>

</xsl:stylesheet>
