#pragma once

namespace GraphicsHelper::Msg
{
    /**
		* @brief D3D12_AUTO_BREADCRUMB_OPのデータをstd::stringに変換する
		*
		* @param  breadcrumbOp - エラーメッセージ
		* @result 変換された文字列
		*/
    inline const std::string BreadCrumbMsgToString(D3D12_AUTO_BREADCRUMB_OP breadcrumbOP)
    {
        std::string msg = utl::str::EnumToString(breadcrumbOP); // enum -> std::string
        utl::str::RemoveWord(msg, "D3D12_AUTO_BREADCRUMB_OP_"); // enumメンバの文字列削除
        return msg;
    }

    /**
		* @brief D3D12_DRED_ALLOCATION_TYPEのデータをstd::stringに変換する
		*
		* @param  dreadAllocationType - エラーメッセージ
		* @result 変換された文字列
		*/
    inline const std::string DredAllocationTypeToString(D3D12_DRED_ALLOCATION_TYPE dredAllocationType)
    {
        std::string msg = utl::str::EnumToString(dredAllocationType); // enum -> std::string
        utl::str::RemoveWord(msg, "D3D12_DRED_ALLOCATION_TYPE_"); // enumメンバの文字列削除
        return msg;
    }
}
