#ifndef QMULTITREE_H
#define QMULTITREE_H

#include <optional>

#include <QtCore/QHash>
#include <QtCore/QScopedPointer>

template <typename TKey, typename TValue>
class QMultiTree
{
public:
	class Node {
	public:
		Node(const Node &other) = delete;
		Node &operator=(const Node &other) = delete;
		Node(Node &&other) noexcept = default;
		Node &operator=(Node &&other) noexcept = default;
		~Node() {
			for (auto child : qAsConst(_children))
				delete child;
		}

		Node *child(const TKey &key) {
			if (!_children.contains(key))
				_children.insert(key, new Node{this});
			return _children[key];
		}
		Node *child(const TKey &key) const {
			return _children.value(key, nullptr);
		}
		Node *find(const QList<TKey> &keys) {
			return find(keys, 0);
		}

		bool hasValue() const {
			return _value.has_value();
		}
		TValue value() const {
			return _value.value_or(TValue{});
		}
		TValue &value() {
			return _value.value();
		}
		void setValue(TValue value) {
			_value = std::move(value);
		}

		int depth() const {
			return _parent ? _parent->depth() + 1 : 0;
		}
		QList<TKey> key() const {
			if (!_parent)
				return {};

			for (auto it = _children.begin(); it != _children.end(); ++it) {
				if (*it == this) {
					auto keyChain = _parent->key();
					keyChain.append(it.key());
					return keyChain;
				}
			}

			return {};
		}
		Node *parent() const {
			return _parent;
		}

		Node *clone() const {
			auto cNode = new Node{nullptr};
			cNode->_value = _value;
			for (auto it = _children.begin(); it != _children.end(); ++it) {
				auto cChild = it->clone();
				cChild->_parent = cNode;
				cNode->_children.insert(it.key(), cChild);
			}
			return cNode;
		}

	private:
		friend class QMultiTree;
		Node *_parent;
		QHash<TKey, Node*> _children;
		std::optional<TValue> _value;

		Node(Node *parent = nullptr) :
			_parent{parent}
		{}

		Node *find(const QList<TKey> &keys, int index) {
			if (index == keys.size())
				return this;
			else
				return child(keys[index])->find(keys, index + 1);
		}
	};

	QMultiTree() :
		_root{new Node{}}
	{}
	QMultiTree(const QMultiTree &other) = delete;
	QMultiTree &operator=(const QMultiTree &other) = delete;
	QMultiTree(QMultiTree &&other) noexcept = default;
	QMultiTree &operator=(QMultiTree &&other) noexcept = default;

	Node &child(const TKey &key) {
		return _root->child(key);
	}
	const Node &child(const TKey &key) const {
		return _root->child(key);
	}

	Node *find(const QList<TKey> &keys) const {
		return _root->find(keys);
	}

	void clear() {
		_root.reset(new Node{});
	}

	QMultiTree clone() {
		QMultiTree cloned;
		cloned._root.reset(_root.clone());
		return cloned;
	}

private:
	QScopedPointer<Node> _root;
};

#endif // QMULTITREE_H
