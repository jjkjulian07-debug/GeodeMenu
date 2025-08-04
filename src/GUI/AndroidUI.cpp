#include "AndroidUI.hpp"
#include "AndroidBall.hpp"
#include "../Client/ExtensionManager.hpp"
#include "Modules/FavouritesTab.hpp"
#include "Modules/SearchBox.hpp"

bool AndroidUI::setup()
{
    bg = BackgroundSprite::create();
    bg->setContentSize(m_size);
    bg->setAnchorPoint(ccp(0, 0));
    bg->updateLayout();

    m_bgSprite->setVisible(false);
    m_buttonMenu->setVisible(false);

    auto backMenu = CCMenu::create();
    backMenu->setPosition(ccp(24, CCDirector::get()->getWinSize().height - 23));
    
    auto backBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(AndroidUI::onClose));
    backMenu->addChild(backBtn);

    populateModules();
    populateTabs();

    m_mainLayer->addChild(bg, -1);
    m_mainLayer->setZOrder(80085);
    this->addChild(backMenu);
    return true;
}

void AndroidUI::populateModules()
{
    categoryMenu = CCNode::create();
    categoryMenu->setContentSize(ccp(475, 280));
    categoryMenu->setID("category-menu");

    for (auto module : Module::moduleMap)
    {
        if (module->getParent())
            continue;

        if (!categories.contains(module->getCategory()))
        {
            auto cat = CategoryNode::getNode(module->getCategory());
            cat->setID(module->getCategory());
            categories.emplace(module->getCategory(), cat);

            categoryMenu->addChildAtPosition(cat, Anchor::Right, ccp(-10, 0));
        }

        categories[module->getCategory()]->addModule(module);
    }

    m_mainLayer->addChild(categoryMenu);
}

void AndroidUI::populateTabs()
{
    auto bg = CCScale9Sprite::create("square02b_small.png");
    bg->setContentSize(ccp(110, m_size.height - 10 * 2) / 0.5f);
    bg->setAnchorPoint(ccp(0, 0.5f));
    bg->setScale(0.5f);
    bg->setColor(ccc3(0, 0, 0));
    bg->setOpacity(100);

    tabsMenu = CCMenu::create();
    tabsMenu->setContentSize((bg->getContentSize() / 2) + ccp(0, -10));
    tabsMenu->setAnchorPoint(ccp(0, 0.5f));
    tabsMenu->ignoreAnchorPointForPosition(false);
    tabsMenu->setLayout(ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setCrossAxisOverflow(true)->setAutoScale(false)->setGap(3.5f));
    tabsMenu->getLayout()->ignoreInvisibleChildren(true);
    
    m_mainLayer->addChildAtPosition(bg, Anchor::Left, ccp(10, 0));
    m_mainLayer->addChildAtPosition(tabsMenu, Anchor::Left, ccp(10 + 5, 0));

    for (auto category : categoryOrders)
    {
        if (category == "spacer")
        {
            for (auto category : ExtensionManager::get()->getCategories())
            {
                addTab(category.name, fmt::format("{}/{}", category.modID, category.name), category.sprite);
            }

            tabsMenu->addChild(geode::SpacerNode::create());

            continue;
        }

        if (!categories.contains(category))
        {
            // meowwww
            auto cat = CategoryNode::getNode(category);
            cat->setID(category);

            categories.emplace(category, cat);
            categoryMenu->addChildAtPosition(cat, Anchor::Right, ccp(-10, 0));
        }

        addTab(category, category, fmt::format("{}{}.png", ""_spr, utils::string::toLower(category)));
    }

    tabsMenu->updateLayout();
    updateTabs();
}

void AndroidUI::addTab(std::string name, std::string id, std::string sprite)
{
    auto sprNormal = CategoryTabSprite::create(CategoryTabType::Text, name, sprite);
    auto sprHeld = CategoryTabSprite::create(CategoryTabType::Text, name, sprite);
    sprHeld->updateSelection(CategorySelectionType::Hovered);

    sprNormal->setContentSize(ccp(100, 20));
    sprHeld->setContentSize(ccp(100, 20));

    auto btn = CCMenuItemSpriteExtra::create(sprNormal, this, menu_selector(AndroidUI::onSelectTab));
    btn->setID(id);
    btn->setSelectedImage(sprHeld);
    btn->m_scaleMultiplier = 1;

    sprNormal->setAnchorPoint(ccp(0, 0));
    sprHeld->setPosition(btn->getContentSize() / 2);

    tabsMenu->addChild(btn);

    categoryBtns[id] = btn;
    categorySprs[id] = sprNormal;
}

void AndroidUI::updateTabs()
{
    for (auto btn : categoryBtns)
    {
        btn.second->setEnabled(selectedCategory != btn.first);
    }
    
    for (auto spr : categorySprs)
    {
        spr.second->updateSelection(selectedCategory != spr.first ? CategorySelectionType::Deselected : CategorySelectionType::Selected);
    }

    for (auto category : categories)
    {
        category.second->setVisible(category.first == selectedCategory);
    }

    categoryBtns["Search"]->setVisible(SearchBox::get()->getRealEnabled());
    categoryBtns["Favourites"]->setVisible(FavouritesTab::get()->getRealEnabled());
    tabsMenu->updateLayout();
}

void AndroidUI::onSelectTab(CCObject* sender)
{
    selectedCategory = as<CCNode*>(sender)->getID();

    updateTabs();
}

AndroidUI* AndroidUI::create()
{
    auto pRet = new AndroidUI();

    if (pRet && pRet->initAnchored(475.f, 280.f))
    {
        PlatformToolbox::showCursor();
        instance = pRet;
        pRet->m_noElasticity = true;

        pRet->autorelease();
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}

AndroidUI* AndroidUI::addToScene()
{
    auto pRet = create();
    pRet->show();

    return pRet;
}

AndroidUI* AndroidUI::get()
{
    return instance;
}

AndroidUI::~AndroidUI()
{
    instance = nullptr;
}

void AndroidUI::close()
{
    if (PlayLayer::get() && !PlayLayer::get()->m_isPaused && !PlayLayer::get()->m_levelEndAnimationStarted && !GameManager::sharedState()->getGameVariable("0024"))
        PlatformToolbox::hideCursor();

    this->onClose(nullptr);
}

void AndroidUI::visit()
{
    AndroidBall::get()->visit();

    geode::Popup<>::visit();
}