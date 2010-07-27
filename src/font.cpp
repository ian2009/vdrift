#include "font.h"

#include "contentmanager.h"
#include "textureloader.h"
#include "texture.h"

#include <algorithm>

bool FONT::Load(
	const std::string & fontinfopath,
	const std::string & fonttexturepath,
	const std::string & texsize,
	ContentManager & content,
	std::ostream & error_output,
	bool mipmap)
{
	std::ifstream fontinfo(fontinfopath.c_str());
	if (!fontinfo)
	{
		error_output << "Can't find font information file: " << fontinfopath << std::endl;
		return false;
	}
	
	std::string curstr;
	while (fontinfo && curstr != "chars")
		fontinfo >> curstr; //advance to first interesting bit
	
	fontinfo >> curstr;
	//if (!VerifyParse("count=96", curstr, fontinfopath, error_output)) return false;
	
	charinfo.resize(128);
	
	while (fontinfo.good())
	{
		fontinfo >> curstr;
		if (fontinfo.good())
		{
			if (!VerifyParse("char", curstr, fontinfopath, error_output)) return false;
			
			unsigned int cur_id(0);
			if (!Parse("id=", cur_id, fontinfo,fontinfopath, error_output)) return false;
			if (cur_id >= charinfo.size())
			{
				error_output << "Font info file " << fontinfopath << ": ID is out of range: " << cur_id << std::endl;
				return false;
			}
			//std::cout << "Parsing ID " << cur_id << endl;
			
			if (!Parse("x=", charinfo[cur_id].x, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("y=", charinfo[cur_id].y, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("width=", charinfo[cur_id].width, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("height=", charinfo[cur_id].height, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("xoffset=", charinfo[cur_id].xoffset, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("yoffset=", charinfo[cur_id].yoffset, fontinfo,fontinfopath, error_output)) return false;
			if (!Parse("xadvance=", charinfo[cur_id].xadvance, fontinfo,fontinfopath, error_output)) return false;
			
			fontinfo >> curstr >> curstr; //don't care
			
			charinfo[cur_id].loaded = true;
		}
	}
	
	TextureLoader texload;
	texload.name = fonttexturepath;
	texload.size = texsize;
	texload.mipmap = mipmap;
	texload.repeatu = false;
	texload.repeatv = false;
	
	font_texture = content.get<TEXTURE>(texload);
	if (!font_texture.get()) return false;
	
	float scale = font_texture->GetScale();
	if (scale != 1.0)
	{
		for (std::vector <CHARINFO>::iterator i = charinfo.begin(); i != charinfo.end(); ++i)
		{
			CHARINFO & char_to_scale = *i;
			char_to_scale.x *= scale;
			char_to_scale.y *= scale;
			char_to_scale.width *= scale;
			char_to_scale.height *= scale;
			char_to_scale.xoffset *= scale;
			char_to_scale.yoffset *= scale;
			char_to_scale.xadvance *= scale;
		}
	}
	
	return true;
}

bool FONT::VerifyParse(
	const std::string & expected,
	const std::string & actual,
	const std::string & fontinfopath,
	std::ostream & error_output) const
{
	if (expected != actual)
	{
		error_output << "Font info file " << fontinfopath << ": expected " << expected << ", got " << actual << std::endl;
		return false;
	}
	else
		return true;
}

float FONT::GetWidth(const std::string & newtext, const float newscale) const
{
	float cursorx(0);

	std::vector <float> linewidth;

	for (unsigned int i = 0; i < newtext.size(); i++)
	{
		if (newtext[i] == '\n')
		{
			linewidth.push_back(cursorx);
			cursorx = 0;
		}
		else
		{
			optional <const FONT::CHARINFO *> cinfo = GetCharInfo(newtext[i]);
			if (cinfo)
				cursorx += (cinfo.get()->xadvance/GetFontTexture()->GetW())*newscale;
		}
	}

	linewidth.push_back(cursorx);

	float maxwidth = *std::max_element(linewidth.begin(), linewidth.end());

	return maxwidth;
}
