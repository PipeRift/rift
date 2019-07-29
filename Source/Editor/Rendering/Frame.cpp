// © 2019 Miguel Fernández Arce - All rights reserved

#include "Frame.h"

u16 Frame::idCounter = 0;


void Frame::ExecuteCommands(FrameRender& render)
{
	ZoneScopedNC("Commands", 0x94d145);
	for (const auto& command : commands)
	{
		command->Execute(render, *this);
	}
	commands.Empty();
}
