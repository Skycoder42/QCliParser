#ifndef QMULTITREE_H
#define QMULTITREE_H

#include <optional>

#include <QtCore/QHash>
#include <QtCore/QSharedPointer>
#include <QtCore/QWeakPointer>

template <typename TKey, typename TValue>
class QMultiTree
{
public:
	class Node {
	private:
		struct Data {
			inline Data(QWeakPointer<Data> parent = {}) :
				parent{parent}
			{}
			inline Data(const Data &) = default;
			inline Data(Data &&) noexcept = default;

			QWeakPointer<Data> parent;
			QHash<TKey, QSharedPointer<Data>> children;
			std::optional<TValue> value;
		};

	public:
		Node() :
			d{QSharedPointer<Data>::create()}
		{}

		Node(const Node &other) = default;
		Node &operator=(const Node &other) = default;
		Node(Node &&other) noexcept = default;
		Node &operator=(Node &&other) noexcept = default;
		~Node() = default;

		explicit operator bool() const {
			return d->parent;
		}
		bool operator!() const {
			return !d->parent;
		}

		Node child(const TKey &key) {
			auto dIter = d->children.find(key);
			if (dIter == d->children.end())
				dIter = d->children.insert(key, QSharedPointer<Data>::create(d.toWeakRef()));
			return *dIter;
		}
		Node child(const TKey &key) const {
			return d->children.value(key, {});
		}
		Node find(const QList<TKey> &keys) {
			return find(keys, 0);
		}

		bool hasValue() const {
			return d->value.has_value();
		}
		TValue value() const {
			return d->value.value_or(TValue{});
		}
		TValue &value() {
			if (!d->value)
				return d->value.emplace();
			else
				return d->value.value();
		}
		void setValue(TValue value) {
			d->value = std::move(value);
		}
		TValue takeValue() {
			if (d->value) {
				auto tValue = std::move(std::move(d->value).value()).value();
				d->value = std::nullopt;
				return tValue;
			} else
				return {};
		}
		void removeValue() {
			d->value = std::nullopt;
		}

		int depth() const {
			const auto parent = d->parent.toStrongRef();
			return parent ? parent->depth() + 1 : 0;
		}
		QList<TKey> key() const {
			const auto parent = d->parent.toStrongRef();
			if (!parent)
				return {};

			for (auto it = d->children.begin(); it != d->children.end(); ++it) {
				if (*it == d) {
					auto keyChain = parent->key();
					keyChain.append(it.key());
					return keyChain;
				}
			}

			return {};
		}
		Node parent() const {
			const auto parent = d->parent.toStrongRef();
			return parent ? Node{parent} : Node{};
		}

		Node *clone() const {
			Node cNode;
			cNode.d->value = d->value;
			for (auto it = d->children.begin(); it != d->children.end(); ++it) {
				// TODO add cloneImpl function to avoid temporary node construction
				auto cChild = Node{*it}.clone().d;
				cChild->parent = cNode.d;
				cNode.d->children.insert(it.key(), cChild);
			}
			return cNode;
		}
	private:
		friend class QMultiTree;
		QSharedPointer<Data> d;

		inline Node(QSharedPointer<Data> data) :
			d{std::move(data)}
		{}

		Node find(const QList<TKey> &keys, int index) {
			if (index == keys.size())
				return *this;
			else // TODO optimize
				return child(keys[index]).find(keys, index + 1);
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
