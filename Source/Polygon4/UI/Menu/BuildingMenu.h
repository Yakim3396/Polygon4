/*
 * Polygon-4
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Menu.h"

namespace polygon4 {
    namespace detail {
        class ModificationMapBuilding;
    }
}

class SBuildingMenu : public SMenu
{
	SLATE_BEGIN_ARGS(SBuildingMenu){}
	SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);

    virtual void OnShow() override;
    virtual void OnHide() override;

    void SetCurrentBuilding(polygon4::detail::ModificationMapBuilding *B);

private:
    TSharedPtr<STextBlock> Name;
    TSharedPtr<STextBlock> Text;

    polygon4::detail::ModificationMapBuilding *CurrentBuilding = nullptr;

    FReply OnExit();
};
