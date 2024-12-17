#pragma once

namespace utl
{
    template<typename OwnerType>
    class StateBase
    {
    public:
        const std::string& GetStateName() const { return m_stateName; }
        void SetStateName(std::string_view _name) { m_stateName = _name; }

        virtual ~StateBase() = default;
        virtual void Enter(OwnerType*   /* _pOwner */) {}
        virtual void Update(OwnerType*  /* _pOwner */) {}
        virtual void Exit(OwnerType*    /* _pOwner */) {}
        // memo : ImGui() はデバッグ用変数なのでオーナーへのアクセスをせず、メンバのみの情報を表示する
        virtual void ImGui(OwnerType* /* _pOwner */) {}

    private:
        std::string m_stateName = "StateBase";
    };

    /**
    * @class StateMachine
    * @brief ステートマシーンを簡易的に扱えるようにしたラッピングクラス
    * @details
    *   このクラスをメンバに持たせることでStateBaseを継承しているオブジェクトの切り替えが行える
    */
    template<typename OwnerType>
    class StateMachine
    {
    public:

        /**
         * @fn const std::string& GetNowStateName() const
         * @brief 現在のステートの名前を取得する
         * @return std::string : 現在のステートの名前
         */
        const std::string& GetNowStateName() const
        {
            if(m_spStateList.empty()) { return "NoState"; }

            return m_spStateList.back()->GetStateName();
        }

        /**
         * @brief 現在のステートを指定した型にキャストして取得する
         * @tparam StateType : 取得したいステートの型
         * @return std::shared_ptr<StateType> : キャストされた現在のステート
         */
        template<typename StateType>
        std::shared_ptr<StateType> GetNowState() const
        {
            if (m_spStateList.empty()) { return nullptr; }
            return std::dynamic_pointer_cast<StateType>(m_spStateList.back());
        }

        /**
         * @fn void Update()
         * @brief 現在のステートを更新する
         */
        void Update()
        {
            if(m_spStateList.empty()) { return; }

            m_spStateList.back()->Update(m_pOwner);
        }

        /**
         * @fn void ImGui()
         * @brief 現在のステートのImGuiを呼び出す
         */
        void ImGui()
        {
            if(m_spStateList.empty()) { return; }

            ImGui::Separator();
            m_spStateList.back()->ImGui(m_pOwner);
        }

        /**
         * @fn void Clean()
         * @brief 現在リストに積まれているステートを全て削除する
         */
        void Clean()
        {
            m_spStateList.clear();
        }

        /**
         * @fn void SetUp(OwnerType* _pOwner)
         * @brief 所有者のポインタを設定する
         * @param _pOwner : 所有者のポインタ - このクラスを持つクラスのthisを渡す
         */
        void SetUp(OwnerType* _pOwner)
        {
            m_pOwner = _pOwner;
        }

        /**
         * @fn void SetState()
         * @brief ステートをリストにスタックする
         * @tparam StateType : 追加したいステートの型
         */
        template<typename StateType>
        std::shared_ptr<StateType> AddState(bool _isPop = false)
        {
            // 所有者が設定されていない場合は追加できないので警告を出す
            if(!m_pOwner) { FNENG_ASSERT_ERROR("StateMachine::SetUp()が呼ばれていません"); return nullptr; }

            // _isPopが有効な場合、現在更新されているステートを削除
            if (_isPop)
            {
                PopState();
            }
            else
            {
                if (!m_spStateList.empty())
                {
                    m_spStateList.back()->Exit(m_pOwner);
                }
            }

            std::shared_ptr<StateBase<OwnerType>> spState = std::make_shared<StateType>();
            spState->SetStateName(typeid(StateType).name());
            spState->Enter(m_pOwner);
            m_spStateList.push_back(spState);

            return std::dynamic_pointer_cast<StateType>(m_spStateList.back());
        }

        /**
         * @fn std::shared_ptr<StateType> ReplaceState()
         * @brief 現在のステートを削除し、新しいステートを追加する
         * @tparam StateType : 追加したいステートの型
         */
        template<typename StateType>
        std::shared_ptr<StateType> ReplaceState()
        {
            if (!m_pOwner)
            {
                FNENG_ASSERT_ERROR("StateMachine::SetUp()が呼ばれていません");
                return nullptr;
            }

            // 現在のステートがあれば削除する
            if (!m_spStateList.empty())
            {
                m_spStateList.back()->Exit(m_pOwner);
                m_spStateList.pop_back();
            }

            // 新しいステートを追加
            std::shared_ptr<StateBase<OwnerType>> spState = std::make_shared<StateType>();
            spState->SetStateName(typeid(StateType).name());
            spState->Enter(m_pOwner);
            m_spStateList.push_back(spState);

            return std::static_pointer_cast<StateType>(m_spStateList.back());
        }

        /**
         * @fn void PopState()
         * @brief 現在更新されているステート(リストの末尾)をリストから取り除く
         */
        void PopState()
        {
            if(m_spStateList.empty()) { return; }

            m_spStateList.back()->Exit(m_pOwner);
            m_spStateList.pop_back();   // 末尾のステートを削除

            // ステートがなくなった場合は何もしない
            if(m_spStateList.empty()) { return; }

            m_spStateList.back()->Enter(m_pOwner);
        }

    private:

        // ステートのリスト : 末尾が現在のステート
        std::list<std::shared_ptr<StateBase<OwnerType>>> m_spStateList;

        OwnerType* m_pOwner = nullptr;
    };
}
