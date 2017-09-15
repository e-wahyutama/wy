#pragma once
#include <wy/menu.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace wy {
	namespace console {
		class menu
			: public wy::menu::menu
		{
		public:
			menu()
				: m_width(2)
				, m_choice("choice: ")
				, m_pause("press any key to continue...")
				, m_invalidInput("invalid input!")
				, m_location("Location: ") {};
			virtual void displayItem(size_t index, const std::string& item) override
			{
				std::cout
					<< std::setw(m_width)
					<< index << ". "
					<< item
					<< std::endl;
			}
			virtual size_t userInput() override
			{
				size_t input = -1;
				std::cout << m_choice;
				if (!(std::cin >> input))
					input = -1; // g++ on non windows need this
				clearCin();
				return input;
			}
			virtual void onBeforeBreadCrumb() override
			{
				std::cout << "Location: ";
			}
			virtual void onAfterBreadCrumb() override
			{
				std::cout << '\n' << std::endl;
			}
			virtual void onBreadCrumbSep(const std::string& str) override
			{
				std::cout << str;
			}
			void onUserInputPrompt(const std::string& prompt)
			{
				this->m_choice = prompt;
			}
			virtual void onPause() override
			{
				std::cout << m_pause;
				std::cin.putback('\n').get();
				clearCin();
			}
			void onPausePrompt(const std::string& prompt)
			{
				this->m_pause = prompt;
			}

			virtual bool onBadInput(size_t badIndex) override
			{
				std::cout << m_invalidInput << std::endl;
				return true;
			}
			void onBadInputPrompt(const std::string& prompt)
			{
				this->m_invalidInput = prompt;
			}

			virtual void initMenu() override
			{
				onInitMenu();
				std::stringstream ss;
				ss << (this->menuSize() + 1);
				m_width = std::max(m_width, ss.str().length());
				setBreadcrumbSeparator(" >> ");
			}

			static void clearScreen()
			{
#ifdef _WIN32
				::system("cls");
#else
				::system("clear");
#endif // _WIN32
			}
			static void clearCin()
			{
				std::cin.clear(); //clear bad input flag
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}

			enum class prompt
			{
				pause,
				choice,
				invalid,
				location
			};

			void setText(prompt where, std::string& str)
			{
				switch (where)
				{
				case prompt::pause:
					m_pause = str;
					break;
				case prompt::choice:
					m_choice = str;
					break;
				case prompt::invalid:
					m_invalidInput = str;
					break;
				case prompt::location:
					m_location = str;
					break;
				}
			}
		private:
			size_t m_width;
			std::string m_pause;
			std::string m_choice;
			std::string m_invalidInput;
			std::string m_location;
		};

	}
}