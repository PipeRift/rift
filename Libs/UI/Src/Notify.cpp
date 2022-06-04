// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/Notify.h"

#include "Math/DateTime.h"
#include "Math/Timespan.h"
#include "UI/Style.h"
#include "UI/UIImgui.h"

#include <Core/String.h>
#include <IconsFontAwesome5.h>
#include <Math/Vector.h>


#define NOTIFY_USE_SEPARATOR


namespace Rift::UI
{
	struct Notification
	{
		enum class Phase : u8
		{
			FadeIn,
			Wait,
			FadeOut,
			Expired
		};
		enum class Corner : u8
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight
		};

		Toast toast;

		DateTime creationTime = DateTime::Now();

		static constexpr float opacity         = 1.f;
		static constexpr float fadeInDuration  = 0.1f;
		static constexpr float fadeOutDuration = 0.15f;
		static constexpr v2 padding{20.f, 20.f};
		static constexpr Corner corner     = Corner::BottomRight;
		static constexpr float separationY = 10.f;    // Padding between notifications
		static constexpr ImGuiWindowFlags flags =
		    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration
		    | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav
		    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;


		Phase GetPhase() const
		{
			const float elapsed = GetElapsedTime().GetTotalSeconds();
			if (elapsed > (fadeInDuration + toast.durationTime + fadeOutDuration))
			{
				return Phase::Expired;
			}
			else if (elapsed > fadeInDuration + toast.durationTime)
			{
				return Phase::FadeOut;
			}
			else if (elapsed > fadeInDuration)
			{
				return Phase::Wait;
			}
			else
			{
				return Phase::FadeIn;
			}
		}

		Timespan GetElapsedTime() const
		{
			return DateTime::Now() - creationTime;
		}

		const char* GetIcon() const
		{
			switch (toast.type)
			{
				case ToastType::Success: return ICON_FA_CHECK_CIRCLE;
				case ToastType::Warning: return ICON_FA_EXCLAMATION_TRIANGLE;
				case ToastType::Error: return ICON_FA_TIMES_CIRCLE;
				case ToastType::Info: return ICON_FA_INFO_CIRCLE;
				case ToastType::None:
				default: return nullptr;
			}
		}

		Color GetColor()
		{
			switch (toast.type)
			{
				case ToastType::Success: return Style::successColor;
				case ToastType::Warning: return Style::warningColor;
				case ToastType::Error: return Style::errorColor;
				case ToastType::Info: return Style::infoColor;
				case ToastType::None:
				default: return Color::White();
			}
		}

		float GetFadePercent() const
		{
			const Phase phase   = GetPhase();
			const float elapsed = GetElapsedTime().GetTotalSeconds();

			if (phase == Phase::FadeIn)
			{
				return (elapsed / Notification::fadeInDuration) * opacity;
			}
			else if (phase == Phase::FadeOut)
			{
				const float invFadeOpacity =
				    (elapsed - Notification::fadeOutDuration - toast.durationTime)
				    / Notification::fadeOutDuration;
				return (1.f - invFadeOpacity) * opacity;
			}
			return 1.f * opacity;
		}

		void GetPositionAndPivot(float height, v2& position, v2& pivot)
		{
			v2 pad{padding.x, padding.y + height};

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			v2 workPos                    = viewport->WorkPos;
			v2 workSize                   = viewport->WorkSize;
			position.x = (u8(corner) & 1) ? (workPos.x + workSize.x - pad.x) : (workPos.x + pad.x);
			position.y = (u8(corner) & 2) ? (workPos.y + workSize.y - pad.y) : (workPos.y + pad.y);
			pivot.x    = (u8(corner) & 1) ? 1.0f : 0.0f;
			pivot.y    = (u8(corner) & 2) ? 1.0f : 0.0f;
		}
	};

	static TArray<Notification> gNotifications;


	void AddNotification(Toast toast)
	{
		gNotifications.Add(Notification{toast});
	}

	void DrawNotifications()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, Style::GetNeutralColor(0));

		float height = 0.f;
		for (i32 i = 0; i < gNotifications.Size(); ++i)
		{
			auto& notification = gNotifications[i];

			// Remove toast if expired
			if (notification.GetPhase() == Notification::Phase::Expired)
			{
				gNotifications.RemoveAt(i);
				--i;
				continue;
			}

			const float opacity   = notification.GetFadePercent();
			LinearColor textColor = notification.GetColor();
			textColor.a           = opacity;

			// ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			ImGui::SetNextWindowBgAlpha(opacity);

			v2 position, pivot;
			notification.GetPositionAndPivot(height, position, pivot);
			ImGui::SetNextWindowPos(position, ImGuiCond_Always, pivot);
			String windowName = Strings::Format("##Notification_{}", i);
			ImGui::Begin(windowName.c_str(), nullptr, Notification::flags);

			// Here we render the toast content
			{
				// We want to support multi-line text, this will
				// wrap the text after 1/3 of the screen width

				const v2 workSize = UI::GetMainViewport()->WorkSize;
				ImGui::PushTextWrapPos(workSize.x / 3.f);

				bool wasTitleRendered = false;

				const char* icon = notification.GetIcon();
				if (icon)
				{
					UI::TextColored(textColor, icon);
					wasTitleRendered = true;
				}

				// If a title is set
				const String& title = notification.toast.title;
				if (!title.empty())
				{
					// If a title and an icon is set, we want to render on same line
					if (icon)
					{
						ImGui::SameLine();
					}

					UI::TextColored(textColor, title);
					wasTitleRendered = true;
				}

				// In case ANYTHING was rendered in the top, we want to add a small padding
				// so the text (or icon) looks centered vertically
				const String& message = notification.toast.message;
				if (wasTitleRendered && !message.empty())
				{
					ImGui::SetCursorPosY(
					    GetCursorPosY() + 5.f);    // Must be a better way to do this!!!!
				}

				// If a content is set
				if (!message.empty())
				{
					if (wasTitleRendered)
					{
#ifdef NOTIFY_USE_SEPARATOR
						ImGui::Separator();
#endif
					}

					UI::Text(message);    // Render content text
				}

				ImGui::PopTextWrapPos();
			}

			// Save height for next toasts
			height += ImGui::GetWindowHeight() + Notification::separationY;

			// End
			ImGui::End();
		}

		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(1);
	}
}    // namespace Rift::UI
