<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://lincity-ng.berlios.de/gui"
	version="1.0">

	<xsl:output method="xml" verions="1.0" encoding="UTF-8" indent="yes"/>

	<xsl:template match="/YesNoDialog">
		<xsl:call-template name="Dialog">
			<xsl:with-param name="image">warning.png</xsl:with-param>
			<xsl:with-param name="buttonbar">
				<xsl:call-template name="yesnobuttonbar"/>
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match="/Invention">
		<xsl:call-template name="Dialog">
			<xsl:with-param name="title">New Invention</xsl:with-param>
			<xsl:with-param name="image">invention.png</xsl:with-param>
			<xsl:with-param name="buttonbar">
				<xsl:call-template name="okbuttonbar"/>
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>

        <xsl:template match="/OkDialog">
            <xsl:call-template name="Dialog">
                <xsl:with-param name="buttonbar">
                    <xsl:call-template name="okbuttonbar"/>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:template>

	<xsl:template name="Dialog">
		<xsl:comment>
WARNING: This file is automatically generated. Do not edit!
</xsl:comment>
		<Window width="700" height="200" border="5" titlesize="25">
			<title>
				<Document>
					<Paragraph style="title" translatable="yes" halign="center" name="DialogTitle">
						<xsl:choose>
							<xsl:when test="title/text()">
								<xsl:value-of select="title/text()"/>
							</xsl:when>                            						
							<xsl:when test="$title">
								<xsl:copy-of select="$title"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:message terminate="yes">
									Dialog Title missing
								</xsl:message>
							</xsl:otherwise>
						</xsl:choose>
					</Paragraph>
				</Document>
			</title>
			<title-background>
				<Gradient from="#ff0000ff" to="#88776655" direction="left-right"/>
			</title-background>
			<background>
				<FilledRectangle color="#ffffffbb"/>
			</background>
			<contents>
				<TableLayout rows="2" cols="1">
					<rowsize row="2" fixed="30"/>
					<cell row="1" col="1">
						<TableLayout rows="1" cols="2">
							<colsize col="1" fixed="95"/>
							<cell row="1" col="1">
								<xsl:element name="Image">
									<xsl:attribute name="src">
										<xsl:text>images/gui/dialogs/</xsl:text>
										<xsl:choose>
											<xsl:when test="image/text()">
												<xsl:value-of select="image/text()"/>
											</xsl:when>
											<xsl:when test="$image">
												<xsl:copy-of select="$image"/>
											</xsl:when>
											<xsl:otherwise>
												<xsl:message terminate="yes">
													Image missing
												</xsl:message>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:attribute>
								</xsl:element>
							</cell>
							<cell row="1" col="2">
								<Document style="message">
									<xsl:if test="bold/text()">
										<p style="messagebold" translatable="yes">
											<xsl:value-of select="bold/text()"/>
										</p>
									</xsl:if>
									<xsl:for-each select="p">
										<xsl:copy>
											<xsl:attribute name="translatable">
												<xsl:text>yes</xsl:text>
											</xsl:attribute>
											<xsl:for-each select="@*">
												<xsl:copy/>
											</xsl:for-each>
											<xsl:value-of select="text()"/>
										</xsl:copy>
									</xsl:for-each>
									<p name="ExtraText"/>
								</Document>
							</cell>
						</TableLayout>
					</cell>
					<cell row="2" col="1">
						<xsl:copy-of select="$buttonbar"/>
					</cell>
				</TableLayout>
			</contents>
		</Window>
	</xsl:template>

	<xsl:template name="okbuttonbar">
		<TableLayout rows="1" cols="1">
			<cell row="1" col="1">
				<Button name="Ok">
					<text-caption style="dialogbutton" translatable="yes">Ok</text-caption>
					<image src="images/gui/dialogs/yesbutton.png"/>
					<image-hover src="images/gui/dialogs/yesbutton-hover.png"/>
					<image-clicked src="images/gui/dialogs/yesbutton-clicked.png"/>
				</Button>
			</cell>
		</TableLayout>
	</xsl:template>

	<xsl:template name="yesnobuttonbar">
		<TableLayout rows="1" cols="2">
			<cell row="1" col="1">
				<Button name="No">
					<text-caption style="dialogbutton" translatable="yes">No</text-caption>
					<image src="images/gui/dialogs/nobutton.png"/>
					<image-hover src="images/gui/dialogs/nobutton-hover.png"/>
					<image-clicked src="images/gui/dialogs/nobutton-clicked.png"/>
				</Button>
			</cell>
			<cell row="1" col="2">
				<Button name="Yes">
					<text-caption style="dialogbutton" translatable="yes">Yes</text-caption>
					<image src="images/gui/dialogs/yesbutton.png"/>
					<image-hover src="images/gui/dialogs/yesbutton-hover.png"/>
					<image-clicked src="images/gui/dialogs/yesbutton-clicked.png"/>
				</Button>
			</cell>
		</TableLayout>
	</xsl:template>
</xsl:stylesheet>

