#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <memory>

namespace wy {
    namespace menu {
        enum class retval
        {
            // quit the menu
            quit_,
            // return to menu
            return_,
            // pause the screen and then return to menu
            pause_,
        };

        namespace {
            class basicItem
            {
            public:
                virtual retval invoke() = 0;
                virtual ~basicItem() {}
            };

            template <typename Method, typename Class>
            class methodItem
                : public basicItem
            {
            public:
                methodItem(Method m, Class h)
                    : m_method(m), m_handler(h) {};

                virtual ~methodItem() {}

                retval invoke() override
                {
                    return (m_handler->*m_method)();
                    //return std::invoke(m_method, m_handler);
                }
            private:
                Method m_method;
                Class m_handler;
            };


            template <typename func>
            class functionItem
                : public basicItem
            {
            public:
                functionItem(func f)
                    : m_functor(f) {}
                retval invoke() override { return m_functor(); }

            private:
                func m_functor;
            };

            // what?
            // I have been forced to use heap,
            // in order to avoid object slicing
            // so here we go, minimal wrapper around the vector
            // with proper deleter
            class storage
            {
            public:
                ~storage() { clear(); }
                void clear()
                {
                    if (m_entries.empty())
                    {
                        for (auto& item : m_entries)
                            delete std::get<1>(item);

                        return m_entries.clear();
                    }
                }
                auto size() const
                {
                    return m_entries.size();
                }
                auto& operator[](size_t idx)
                {
                    return m_entries[idx];
                }
                auto begin()
                {
                    return m_entries.begin();
                }
                auto end()
                {
                    return m_entries.end();
                }
                void push(const std::string& str, basicItem* item)
                {
                    m_entries.push_back(std::make_tuple(str, item));
                }
                auto empty()
                {
                    return m_entries.empty();
                }
            private:
                std::vector<std::tuple<std::string, basicItem*>> m_entries;

            };
        }
        class menu
        {
        public:

            typedef retval(menu::*Entry)();

            menu()
                : m_parent(nullptr)
            {
                setExitMenuText("Exit");
            }
            virtual ~menu() { clearMenu(); }
            retval menuExit() { return retval::quit_; }

            void clearMenu() { m_storage.clear(); }

            void setParent(menu& parent) { m_parent = &parent; }

            void setBreadcrumbSeparator(const std::string& sep) { m_sep = sep; }
            virtual bool onBreadCrumb() { return false; };
            virtual void onBeforeBreadCrumb() {};
            virtual void onAfterBreadCrumb() {};
            virtual void onBreadCrumbSep(const std::string& str) {};
            bool breadcrumb()
            {
                if (m_parent)
                {
                    if (m_parent->breadcrumb())
                        onBreadCrumbSep(m_sep);
                }

                return onBreadCrumb();
            }

            virtual void displayItem(size_t index, const std::string& item) = 0;
            virtual size_t userInput() = 0;
            virtual void onPause() = 0;

            // return true to continue...
            virtual bool onBadInput(size_t badIndex) = 0;
            virtual void onStartMenu() {};
            virtual void onExitMenu() {};

            virtual void onInitMenu() = 0;
            // override this method to initialize a menu,
            // but remember to call onInitMenu
            virtual void initMenu() { return onInitMenu(); };

            size_t menuSize() { return m_storage.size(); }

            void display()
            {
                size_t init = menuSize() + 1;
                size_t input = init;
                do {
                    this->onStartMenu();
                    _breadcrumb();

                    // 0 is reserved for exit(prev menu if any)
                    size_t index = 0;
                    for (auto& item : m_storage)
                    {
                        if (index != 0)
                            this->displayItem(index, std::get<0>(item));
                        index++;
                    }
                    this->displayItem(0, std::get<0>(m_storage[0]));

                    // input
                    input = init;
                    input = this->userInput();

                    if (input >= 0 && input < m_storage.size())
                    {

                        auto& tuple = m_storage[input];
                        auto& invokeMe = std::get<1>(tuple);

                        switch (invokeMe->invoke())
                        {
                        case retval::quit_:
                            this->onExitMenu();
                            input = 0;
                            break;
                        case retval::pause_:
                            this->onPause();
                            // --fallthrough--
                        case retval::return_:
                            break;

                        }
                    }
                    else
                    {
                        // bad input
                        if (false == this->onBadInput(input))
                        {
                            this->onExitMenu();
                            break;
                        }
                        this->onPause();
                        input = init;
                    }
                } while (input != 0);
            }

            template <typename Method, typename Class>
            void push(const std::string& menuText, Method&& entry, Class cv)
            {
                auto item = new methodItem<Method, Class>(entry, cv);
                m_storage.push(menuText, item);
            }

            template <typename functor>
            void push(const std::string& menuText, functor f)
            {
                auto item = new functionItem<functor>(f);
                m_storage.push(menuText, item);
            }
        protected:
            void setExitMenuText(const std::string& exitStr)
            {
                if (!(m_storage.empty()))
                {
                    std::get<0>(m_storage[0]) = exitStr;
                    return;
                }
                push(exitStr, &menu::menuExit, this);
            }
        private:
            void _breadcrumb()
            {
                this->onBeforeBreadCrumb();
                this->breadcrumb();
                this->onAfterBreadCrumb();
            }
            menu* m_parent;
            std::string m_sep;
            storage m_storage;
        };
    };
};
using menu_retval = wy::menu::retval;
